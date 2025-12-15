pipeline {
    agent any
    
    environment {
        DOCKER_IMAGE = "casino-game"
        DOCKER_TAG = "${env.BUILD_NUMBER}"
        CONTAINER_NAME = "casino-game-container"
    }
    
    stages {
        stage('Build') {
            steps {
                sh 'rm -rf build'
                sh 'cmake -B build -S .'
                sh 'cmake --build build'
            }
        }
        
        stage('Static Analysis') {
            parallel {
                stage('Cppcheck - Code Quality') {
                    steps {
                        sh '''
                            # Install cppcheck if not available
                            which cppcheck || sudo apt-get install -y cppcheck
                            
                            # Run cppcheck with all checks enabled
                            cppcheck --enable=all \
                                     --inconclusive \
                                     --xml \
                                     --xml-version=2 \
                                     --suppress=missingIncludeSystem \
                                     src/ 2> cppcheck-report.xml
                            
                            # Convert XML to text report
                            cppcheck --enable=all \
                                     --inconclusive \
                                     --suppress=missingIncludeSystem \
                                     src/ 2> cppcheck-report.txt
                        '''
                        publishCppcheck pattern: 'cppcheck-report.xml'
                        archiveArtifacts artifacts: 'cppcheck-report.txt', allowEmptyArchive: true
                    }
                }
                
                stage('Clang-Tidy - Linting') {
                    steps {
                        sh '''
                            # Install clang-tidy if not available
                            which clang-tidy || sudo apt-get install -y clang-tidy
                            
                            # Run clang-tidy on all C++ files
                            find src/ -name '*.cpp' -exec clang-tidy {} -- -std=c++11 \\; > clang-tidy-report.txt || true
                        '''
                        archiveArtifacts artifacts: 'clang-tidy-report.txt', allowEmptyArchive: true
                    }
                }
                
                stage('Flawfinder - Security Scan') {
                    steps {
                        sh '''
                            # Install flawfinder if not available
                            which flawfinder || pip install flawfinder
                            
                            # Run flawfinder security scanner
                            flawfinder --html --minlevel=0 src/ > flawfinder-report.html
                            flawfinder --minlevel=0 src/ > flawfinder-report.txt
                            flawfinder --sarif src/ > flawfinder-report.sarif
                        '''
                        publishHTML([
                            reportDir: '.',
                            reportFiles: 'flawfinder-report.html',
                            reportName: 'Flawfinder Security Report'
                        ])
                        archiveArtifacts artifacts: 'flawfinder-report.txt,flawfinder-report.sarif', allowEmptyArchive: true
                    }
                }
            }
        }
        
        stage('Test') {
            steps {
                sh '''
                    export JENKINS_HOME=/tmp
                    ./build/casino_game
                    ./build/test_game
                '''
            }
        }
        
        stage('Code Coverage') {
            steps {
                sh '''
                    # Rebuild with coverage flags
                    cmake -B build-coverage -S . -DCMAKE_CXX_FLAGS="--coverage"
                    cmake --build build-coverage
                    
                    # Run tests with coverage
                    export JENKINS_HOME=/tmp
                    ./build-coverage/test_game
                    
                    # Generate coverage report
                    which gcovr || pip install gcovr
                    gcovr -r . --html --html-details -o coverage-report.html
                    gcovr -r . --txt -o coverage-report.txt
                '''
                publishHTML([
                    reportDir: '.',
                    reportFiles: 'coverage-report.html',
                    reportName: 'Code Coverage Report'
                ])
                archiveArtifacts artifacts: 'coverage-report.txt', allowEmptyArchive: true
            }
        }
        
        stage('Build Docker Image') {
            steps {
                script {
                    sh '''
                        # Create Dockerfile if it doesn't exist
                        cat > Dockerfile << 'EOF'
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy built executables
COPY build/casino_game /app/
COPY build/test_game /app/

# Set executable permissions
RUN chmod +x /app/casino_game /app/test_game

# Set environment variable for non-interactive mode
ENV JENKINS_HOME=/tmp

# Add health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD test -f /app/casino_game || exit 1

# Default command
CMD ["/app/casino_game"]
EOF
                        
                        # Build Docker image
                        docker build -t ${DOCKER_IMAGE}:${DOCKER_TAG} .
                        docker tag ${DOCKER_IMAGE}:${DOCKER_TAG} ${DOCKER_IMAGE}:latest
                    '''
                }
            }
        }
        
        stage('Docker Image Vulnerability Scan') {
            parallel {
                stage('Trivy Scan') {
                    steps {
                        sh '''
                            # Install Trivy if not available
                            if ! which trivy; then
                                wget -qO - https://aquasecurity.github.io/trivy-repo/deb/public.key | sudo apt-key add -
                                echo "deb https://aquasecurity.github.io/trivy-repo/deb $(lsb_release -sc) main" | sudo tee -a /etc/apt/sources.list.d/trivy.list
                                sudo apt-get update && sudo apt-get install -y trivy
                            fi
                            
                            # Update Trivy database
                            trivy image --download-db-only
                            
                            # Scan Docker image - JSON format
                            trivy image --format json --output trivy-report.json ${DOCKER_IMAGE}:${DOCKER_TAG}
                            
                            # Scan Docker image - Table format (text)
                            trivy image --format table --output trivy-report.txt ${DOCKER_IMAGE}:${DOCKER_TAG}
                            
                            # Scan with severity threshold
                            trivy image --severity HIGH,CRITICAL --format table --output trivy-critical-report.txt ${DOCKER_IMAGE}:${DOCKER_TAG}
                            
                            # Generate HTML report
                            trivy image --format template --template "@contrib/html.tpl" --output trivy-report.html ${DOCKER_IMAGE}:${DOCKER_TAG}
                        '''
                        publishHTML([
                            reportDir: '.',
                            reportFiles: 'trivy-report.html',
                            reportName: 'Trivy Vulnerability Report'
                        ])
                        archiveArtifacts artifacts: 'trivy-report.txt,trivy-critical-report.txt,trivy-report.json', allowEmptyArchive: true
                    }
                }
                
                stage('Grype Scan') {
                    steps {
                        sh '''
                            # Install Grype if not available
                            if ! which grype; then
                                curl -sSfL https://raw.githubusercontent.com/anchore/grype/main/install.sh | sh -s -- -b /usr/local/bin
                            fi
                            
                            # Scan with Grype - JSON format
                            grype ${DOCKER_IMAGE}:${DOCKER_TAG} -o json > grype-report.json
                            
                            # Scan with Grype - Table format (text)
                            grype ${DOCKER_IMAGE}:${DOCKER_TAG} -o table > grype-report.txt
                            
                            # Scan with Grype - SARIF format
                            grype ${DOCKER_IMAGE}:${DOCKER_TAG} -o sarif > grype-report.sarif
                        '''
                        archiveArtifacts artifacts: 'grype-report.txt,grype-report.json,grype-report.sarif', allowEmptyArchive: true
                    }
                }
            }
        }
        
        stage('Deploy Docker Container') {
            steps {
                script {
                    sh '''
                        # Stop and remove existing container if running
                        docker stop ${CONTAINER_NAME} || true
                        docker rm ${CONTAINER_NAME} || true
                        
                        # Run the container in detached mode
                        docker run -d \
                            --name ${CONTAINER_NAME} \
                            --restart unless-stopped \
                            ${DOCKER_IMAGE}:${DOCKER_TAG}
                        
                        # Wait for container to start
                        sleep 5
                    '''
                }
            }
        }
        
        stage('Container Health Check') {
            steps {
                script {
                    sh '''
                        echo "=== Container Health Check Report ===" > container-health-report.txt
                        echo "Build Number: ${BUILD_NUMBER}" >> container-health-report.txt
                        echo "Timestamp: $(date)" >> container-health-report.txt
                        echo "" >> container-health-report.txt
                        
                        # Check container status
                        echo "Container Status:" >> container-health-report.txt
                        docker ps -a --filter "name=${CONTAINER_NAME}" --format "table {{.Names}}\t{{.Status}}\t{{.State}}" >> container-health-report.txt
                        echo "" >> container-health-report.txt
                        
                        # Check if container is running
                        CONTAINER_STATUS=$(docker inspect -f '{{.State.Status}}' ${CONTAINER_NAME})
                        echo "Container State: $CONTAINER_STATUS" >> container-health-report.txt
                        
                        if [ "$CONTAINER_STATUS" != "running" ]; then
                            echo "ERROR: Container is not running!" >> container-health-report.txt
                            exit 1
                        fi
                        
                        # Check health status
                        HEALTH_STATUS=$(docker inspect -f '{{.State.Health.Status}}' ${CONTAINER_NAME} 2>/dev/null || echo "No health check defined")
                        echo "Health Status: $HEALTH_STATUS" >> container-health-report.txt
                        echo "" >> container-health-report.txt
                        
                        # Get container logs
                        echo "Container Logs (last 50 lines):" >> container-health-report.txt
                        docker logs --tail 50 ${CONTAINER_NAME} >> container-health-report.txt 2>&1
                        echo "" >> container-health-report.txt
                        
                        # Resource usage
                        echo "Resource Usage:" >> container-health-report.txt
                        docker stats --no-stream ${CONTAINER_NAME} >> container-health-report.txt
                        echo "" >> container-health-report.txt
                        
                        # Network information
                        echo "Network Information:" >> container-health-report.txt
                        docker inspect -f '{{range .NetworkSettings.Networks}}IP: {{.IPAddress}}{{end}}' ${CONTAINER_NAME} >> container-health-report.txt
                        
                        echo "" >> container-health-report.txt
                        echo "=== Health Check Completed Successfully ===" >> container-health-report.txt
                    '''
                    archiveArtifacts artifacts: 'container-health-report.txt'
                }
            }
        }
        
        stage('Deliver') {
            steps {
                sh '''
                    # Archive the built executable
                    tar -czf casino_game.tar.gz build/casino_game
                    
                    # Save Docker image as tar
                    docker save ${DOCKER_IMAGE}:${DOCKER_TAG} -o ${DOCKER_IMAGE}-${DOCKER_TAG}.tar
                    gzip ${DOCKER_IMAGE}-${DOCKER_TAG}.tar
                    
                    # Create delivery summary report
                    cat > delivery-summary.txt << EOF
=== Delivery Summary Report ===
Build Number: ${BUILD_NUMBER}
Build Date: $(date)
Docker Image: ${DOCKER_IMAGE}:${DOCKER_TAG}
Container Name: ${CONTAINER_NAME}

Artifacts:
- casino_game.tar.gz (Binary)
- ${DOCKER_IMAGE}-${DOCKER_TAG}.tar.gz (Docker Image)

Security Scan Results:
- Trivy Report: trivy-report.txt
- Grype Report: grype-report.txt

Code Quality Reports:
- Cppcheck: cppcheck-report.txt
- Clang-Tidy: clang-tidy-report.txt
- Flawfinder: flawfinder-report.txt
- Coverage: coverage-report.txt

Container Status: Running
Health Check: container-health-report.txt

=== Delivery Completed ===
EOF
                '''
                archiveArtifacts artifacts: 'casino_game.tar.gz,*-*.tar.gz,delivery-summary.txt', fingerprint: true
            }
        }
    }
    
    post {
        always {
            // Archive all analysis reports as text files
            archiveArtifacts artifacts: '*-report.txt,*-report.json,*-report.sarif', allowEmptyArchive: true
            
            // Clean up old containers (optional)
            sh '''
                # Keep only last 3 containers
                docker ps -a --filter "name=${DOCKER_IMAGE}" --format "{{.Names}}" | tail -n +4 | xargs -r docker rm -f
            '''
        }
        success {
            echo 'Pipeline completed successfully! All reports archived.'
        }
        failure {
            echo 'Pipeline failed! Check the reports for details.'
            sh 'docker logs ${CONTAINER_NAME} || true'
        }
    }
}
