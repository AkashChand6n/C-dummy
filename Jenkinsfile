pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh 'rm -rf build'
                sh 'cmake -B build -S .'
                sh 'cmake --build build'
            }
        }
        stage('Test') {
            steps {
                sh './build/casino_game'
                sh './build/test_game'
            }
        }
        stage('Deliver') {
            steps {
                sh 'tar -czf casino_game.tar.gz build/casino_game'
                archiveArtifacts artifacts: 'casino_game.tar.gz', fingerprint: true
            }
        }
    }
}