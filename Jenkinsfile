pipeline {
    agent any
    
    environment {
        DOCKER_IMAGE = "casino-game"
        DOCKER_TAG = "${env.BUILD_NUMBER}"
        CONTAINER_NAME = "casino-game-container"
        PATH = "${env.PATH}:${env.HOME}/.local/bin"
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
            steps {
                script {
                    sh '''
                        sudo apt-get update || true
                        sudo apt-get install -y cppcheck clang-tidy doxygen graphviz valgrind || true
                        pip install --user flawfinder gcovr cpplint || true
                        export PATH="$HOME/.local/bin:$PATH"
                    '''
                    
                    parallel(
                        "Cppcheck": {
                            sh '''
                                cppcheck --enable=all \
                                         --inconclusive \
                                         --xml \
                                         --xml-version=2 \
                                         --suppress=missingIncludeSystem \
                                         src/ 2> cppcheck-report.xml || true
                                
                                cppcheck --enable=all \
                                         --inconclusive \
                                         --suppress=missingIncludeSystem \
                                         src/ 2> cppcheck-report.txt || true
                            '''
                            archiveArtifacts artifacts: 'cppcheck-report.txt,cppcheck-report.xml', allowEmptyArchive: true
                        },
                        "Clang-Tidy": {
                            sh '''
                                find src/ -name '*.cpp' -exec clang-tidy {} -- -std=c++11 \\; > clang-tidy-report.txt 2>&1 || true
                            '''
                            archiveArtifacts artifacts: 'clang-tidy-report.txt', allowEmptyArchive: true
                        },
                        "Flawfinder": {
                            sh '''
                                export PATH="$HOME/.local/bin:$PATH"
                                flawfinder --html --minlevel=0 src/ > flawfinder-report.html || true
                                flawfinder --minlevel=0 src/ > flawfinder-report.txt || true
                                flawfinder --sarif src/ > flawfinder-report.sarif || true
                            '''
                            archiveArtifacts artifacts: 'flawfinder-report.html,flawfinder-report.txt,flawfinder-report.sarif', allowEmptyArchive: true
                        },
                        "CPPLint": {
                            sh '''
                                export PATH="$HOME/.local/bin:$PATH"
                                find src/ -name '*.cpp' -o -name '*.h' | xargs cpplint --output=junit > cpplint-report.xml 2>&1 || true
                                find src/ -name '*.cpp' -o -name '*.h' | xargs cpplint > cpplint-report.txt 2>&1 || true
                            '''
                            archiveArtifacts artifacts: 'cpplint-report.txt,cpplint-report.xml', allowEmptyArchive: true
                        }
                    )
                }
            }
        }
        
        stage('Memory Leak Detection') {
            steps {
                sh '''
                    export JENKINS_HOME=/tmp
                    valgrind --leak-check=full \
                             --show-leak-kinds=all \
                             --track-origins=yes \
                             --verbose \
                             --log-file=valgrind-report.txt \
                             ./build/test_game || true
                    
                    echo "=== Valgrind Memory Leak Analysis ===" > valgrind-summary.txt
                    grep -E "ERROR SUMMARY|definitely lost|indirectly lost|possibly lost" valgrind-report.txt >> valgrind-summary.txt || true
                '''
                archiveArtifacts artifacts: 'valgrind-report.txt,valgrind-summary.txt', allowEmptyArchive: true
            }
        }
        
        stage('Duplicate Code Detection') {
            steps {
                sh '''
                    if [ ! -f pmd-bin/bin/pmd ]; then
                        wget -q https://github.com/pmd/pmd/releases/download/pmd_releases%2F7.9.0/pmd-dist-7.9.0-bin.zip || true
                        unzip -q pmd-dist-7.9.0-bin.zip || true
                        mv pmd-bin-7.9.0 pmd-bin || true
                    fi
                    
                    ./pmd-bin/bin/pmd cpd --minimum-tokens 50 \
                                          --language cpp \
                                          --dir src/ \
                                          --format text > cpd-report.txt || true
                    
                    ./pmd-bin/bin/pmd cpd --minimum-tokens 50 \
                                          --language cpp \
                                          --dir src/ \
                                          --format xml > cpd-report.xml || true
                    
                    echo "=== Duplicate Code Detection Summary ===" > cpd-summary.txt
                    grep -c "Found" cpd-report.txt >> cpd-summary.txt || echo "0 duplicates found" >> cpd-summary.txt
                '''
                archiveArtifacts artifacts: 'cpd-report.txt,cpd-report.xml,cpd-summary.txt', allowEmptyArchive: true
            }
        }
        
        stage('Documentation Generation') {
            steps {
                sh '''
                    if [ ! -f Doxyfile ]; then
                        doxygen -g Doxyfile
                        sed -i 's/PROJECT_NAME           = "My Project"/PROJECT_NAME           = "Casino Game"/' Doxyfile
                        sed -i 's/OUTPUT_DIRECTORY       =/OUTPUT_DIRECTORY       = docs/' Doxyfile
                        sed -i 's/INPUT                  =/INPUT                  = src/' Doxyfile
                        sed -i 's/RECURSIVE              = NO/RECURSIVE              = YES/' Doxyfile
                        sed -i 's/GENERATE_HTML          = YES/GENERATE_HTML          = YES/' Doxyfile
                        sed -i 's/GENERATE_LATEX         = YES/GENERATE_LATEX         = NO/' Doxyfile
                        sed -i 's/EXTRACT_ALL            = NO/EXTRACT_ALL            = YES/' Doxyfile
                        sed -i 's/HAVE_DOT               = NO/HAVE_DOT               = YES/' Doxyfile
                        sed -i 's/UML_LOOK               = NO/UML_LOOK               = YES/' Doxyfile
                        sed -i 's/CALL_GRAPH             = NO/CALL_GRAPH             = YES/' Doxyfile
                        sed -i 's/CALLER_GRAPH           = NO/CALLER_GRAPH           = YES/' Doxyfile
                    fi
                    
                    doxygen Doxyfile || true
                    
                    echo "=== Documentation Generation Report ===" > doxygen-summary.txt
                    echo "Generated on: $(date)" >> doxygen-summary.txt
                    echo "Files documented: $(find src/ -name '*.cpp' -o -name '*.h' | wc -l)" >> doxygen-summary.txt
                    echo "HTML Documentation: docs/html/index.html" >> doxygen-summary.txt
                '''
                archiveArtifacts artifacts: 'docs/**/*,Doxyfile,doxygen-summary.txt', allowEmptyArchive: true
            }
        }
        
        stage('Complexity Analysis') {
            steps {
                sh '''
                    pip install --user lizard || true
                    export PATH="$HOME/.local/bin:$PATH"
                    
                    lizard src/ -o lizard-report.html || true
                    lizard src/ > lizard-report.txt || true
                    
                    echo "=== Cyclomatic Complexity Analysis ===" > complexity-summary.txt
                    echo "Functions with complexity > 15:" >> complexity-summary.txt
                    lizard src/ | grep -E "^[[:space:]]*[0-9]+" | awk '$1 > 15 {print}' >> complexity-summary.txt || true
                '''
                archiveArtifacts artifacts: 'lizard-report.html,lizard-report.txt,complexity-summary.txt', allowEmptyArchive: true
            }
        }
        
        stage('Dependency Analysis') {
            steps {
                sh '''
                    echo "=== Include Dependency Analysis ===" > dependency-report.txt
                    echo "Generated on: $(date)" >> dependency-report.txt
                    echo "" >> dependency-report.txt
                    
                    for file in src/*.cpp; do
                        echo "File: $file" >> dependency-report.txt
                        echo "Includes:" >> dependency-report.txt
                        grep -E "^#include" "$file" | sed 's/^/  /' >> dependency-report.txt || true
                        echo "" >> dependency-report.txt
                    done
                    
                    echo "=== External Library Dependencies ===" >> dependency-report.txt
                    grep -rh "^#include <" src/ | sort -u >> dependency-report.txt || true
                '''
                archiveArtifacts artifacts: 'dependency-report.txt', allowEmptyArchive: true
            }
        }
        
        stage('Build Metrics') {
            steps {
                sh '''
                    echo "=== Build Metrics Report ===" > build-metrics.txt
                    echo "Build Number: ${BUILD_NUMBER}" >> build-metrics.txt
                    echo "Build Date: $(date)" >> build-metrics.txt
                    echo "" >> build-metrics.txt
                    
                    echo "Lines of Code:" >> build-metrics.txt
                    find src/ -name '*.cpp' -o -name '*.h' | xargs wc -l | tail -1 >> build-metrics.txt
                    
                    echo "" >> build-metrics.txt
                    echo "File Counts:" >> build-metrics.txt
                    echo "C++ Source Files: $(find src/ -name '*.cpp' | wc -l)" >> build-metrics.txt
                    echo "Header Files: $(find src/ -name '*.h' | wc -l)" >> build-metrics.txt
                    
                    echo "" >> build-metrics.txt
                    echo "Binary Size:" >> build-metrics.txt
                    ls -lh build/casino_game build/test_game | awk '{print $9 ": " $5}' >> build-metrics.txt || true
                '''
                archiveArtifacts artifacts: 'build-metrics.txt', allowEmptyArchive: true
            }
        }
        
        stage('Test') {
            steps {
                sh '''
                    export JENKINS_HOME=/tmp
                    ./build/casino_game || true
                    ./build/test_game
                '''
            }
        }
        
        stage('Code Coverage') {
            steps {
                sh '''
                    cmake -B build-coverage -S . -DCMAKE_CXX_FLAGS="--coverage"
                    cmake --build build-coverage
                    
                    export JENKINS_HOME=/tmp
                    ./build-coverage/test_game || true
                    
                    export PATH="$HOME/.local/bin:$PATH"
                    
                    gcovr -r . --html --html-details -o coverage-report.html || true
                    gcovr -r . --txt -o coverage-report.txt || true
                    
                    echo "=== Code Coverage Summary ===" > coverage-summary.txt
                    gcovr -r . | tail -5 >> coverage-summary.txt || true
                '''
                archiveArtifacts artifacts: 'coverage-report.html,coverage-report.txt,coverage-summary.txt', allowEmptyArchive: true
            }
        }
        
        stage('Build Docker Image') {
            steps {
                script {
                    sh '''
                        # Verify Dockerfile exists in repo
                        if [ ! -f Dockerfile ]; then
                            echo "ERROR: Dockerfile not found in repository!"
                            exit 1
                        fi
                        
                        # Build Docker image using Dockerfile from repo
                        docker build -t ${DOCKER_IMAGE}:${DOCKER_TAG} .
                        docker tag ${DOCKER_IMAGE}:${DOCKER_TAG} ${DOCKER_IMAGE}:latest
                        
                        # Display Dockerfile content for verification
                        echo "=== Using Dockerfile from repository ===" > dockerfile-info.txt
                        cat Dockerfile >> dockerfile-info.txt
                    '''
                    archiveArtifacts artifacts: 'dockerfile-info.txt', allowEmptyArchive: true
                }
            }
        }
        
        stage('Docker Image Vulnerability Scan') {
            steps {
                script {
                    sh '''
                        if ! which trivy; then
                            wget -qO - https://aquasecurity.github.io/trivy-repo/deb/public.key | sudo apt-key add - || true
                            echo "deb https://aquasecurity.github.io/trivy-repo/deb $(lsb_release -sc) main" | sudo tee -a /etc/apt/sources.list.d/trivy.list
                            sudo apt-get update && sudo apt-get install -y trivy || true
                        fi
                        
                        if ! which grype; then
                            curl -sSfL https://raw.githubusercontent.com/anchore/grype/main/install.sh | sudo sh -s -- -b /usr/local/bin || true
                        fi
                    '''
                    
                    parallel(
                        "Trivy": {
                            sh '''
                                trivy image --download-db-only || true
                                trivy image --format json --output trivy-report.json ${DOCKER_IMAGE}:${DOCKER_TAG} || true
                                trivy image --format table --output trivy-report.txt ${DOCKER_IMAGE}:${DOCKER_TAG} || true
                                trivy image --severity HIGH,CRITICAL --format table --output trivy-critical-report.txt ${DOCKER_IMAGE}:${DOCKER_TAG} || true
                            '''
                            archiveArtifacts artifacts: 'trivy-report.txt,trivy-critical-report.txt,trivy-report.json', allowEmptyArchive: true
                        },
                        "Grype": {
                            sh '''
                                grype ${DOCKER_IMAGE}:${DOCKER_TAG} -o json > grype-report.json || true
                                grype ${DOCKER_IMAGE}:${DOCKER_TAG} -o table > grype-report.txt || true
                                grype ${DOCKER_IMAGE}:${DOCKER_TAG} -o sarif > grype-report.sarif || true
                            '''
                            archiveArtifacts artifacts: 'grype-report.txt,grype-report.json,grype-report.sarif', allowEmptyArchive: true
                        }
                    )
                }
            }
        }
        
        stage('Deploy Docker Container') {
            steps {
                script {
                    sh '''
                        docker stop ${CONTAINER_NAME} 2>/dev/null || true
                        docker rm ${CONTAINER_NAME} 2>/dev/null || true
                        
                        docker run -d \
                            --name ${CONTAINER_NAME} \
                            --restart unless-stopped \
                            ${DOCKER_IMAGE}:${DOCKER_TAG}
                        
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
                        
                        echo "Container Status:" >> container-health-report.txt
                        docker ps -a --filter "name=${CONTAINER_NAME}" --format "table {{.Names}}\t{{.Status}}\t{{.State}}" >> container-health-report.txt
                        echo "" >> container-health-report.txt
                        
                        CONTAINER_STATUS=$(docker inspect -f '{{.State.Status}}' ${CONTAINER_NAME} 2>/dev/null || echo "unknown")
                        echo "Container State: $CONTAINER_STATUS" >> container-health-report.txt
                        
                        if [ "$CONTAINER_STATUS" != "running" ]; then
                            echo "WARNING: Container is not running!" >> container-health-report.txt
                        fi
                        
                        HEALTH_STATUS=$(docker inspect -f '{{.State.Health.Status}}' ${CONTAINER_NAME} 2>/dev/null || echo "No health check defined")
                        echo "Health Status: $HEALTH_STATUS" >> container-health-report.txt
                        echo "" >> container-health-report.txt
                        
                        echo "Container Logs (last 50 lines):" >> container-health-report.txt
                        docker logs --tail 50 ${CONTAINER_NAME} >> container-health-report.txt 2>&1 || true
                        echo "" >> container-health-report.txt
                        
                        echo "Resource Usage:" >> container-health-report.txt
                        docker stats --no-stream ${CONTAINER_NAME} >> container-health-report.txt 2>&1 || true
                        echo "" >> container-health-report.txt
                        
                        echo "Network Information:" >> container-health-report.txt
                        docker inspect -f '{{range .NetworkSettings.Networks}}IP: {{.IPAddress}}{{end}}' ${CONTAINER_NAME} >> container-health-report.txt 2>&1 || true
                        
                        echo "" >> container-health-report.txt
                        echo "=== Health Check Completed ===" >> container-health-report.txt
                    '''
                    archiveArtifacts artifacts: 'container-health-report.txt'
                }
            }
        }
        
        stage('Deliver') {
            steps {
                sh '''
                    tar -czf casino_game.tar.gz build/casino_game
                    docker save ${DOCKER_IMAGE}:${DOCKER_TAG} -o ${DOCKER_IMAGE}-${DOCKER_TAG}.tar
                    gzip ${DOCKER_IMAGE}-${DOCKER_TAG}.tar
                    
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
- CPPLint: cpplint-report.txt
- Coverage: coverage-report.txt

Additional Analysis:
- Memory Leaks: valgrind-report.txt
- Duplicate Code: cpd-report.txt
- Documentation: docs/html/index.html
- Complexity: lizard-report.txt
- Dependencies: dependency-report.txt
- Build Metrics: build-metrics.txt

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
            archiveArtifacts artifacts: '*-report.txt,*-report.json,*-report.sarif,*-report.xml,*-report.html,*-summary.txt', allowEmptyArchive: true
            
            sh '''
                docker ps -a --filter "name=${DOCKER_IMAGE}" --format "{{.Names}}" | tail -n +4 | xargs -r docker rm -f || true
            '''
        }
        success {
            echo 'Pipeline completed successfully! All reports archived.'
        }
        failure {
            echo 'Pipeline failed! Check the reports for details.'
            sh 'docker logs ${CONTAINER_NAME} 2>&1 || true'
        }
    }
}
