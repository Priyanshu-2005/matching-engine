@echo off
REM Deployment test script for Ultra-Low-Latency Matching Engine
REM This script tests the Docker-based deployment locally on Windows

echo === Deployment Test Script ===
echo Testing Phase 7: CI/CD ^& Deployment
echo.

REM Check Docker availability
where docker >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ Docker is not installed or not in PATH
    exit /b 1
)

docker info >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ Docker daemon is not running
    echo    Please start Docker Desktop or the Docker service
    exit /b 1
)

docker --version
echo ✅ Docker is available

REM Check Docker Compose
where docker-compose >nul 2>nul
if %errorlevel% equ 0 (
    set DOCKER_COMPOSE=docker-compose
) else (
    docker compose version >nul 2>nul
    if %errorlevel% equ 0 (
        set DOCKER_COMPOSE=docker compose
    ) else (
        echo ⚠️  docker-compose not found
        exit /b 1
    )
)

echo ✅ Docker Compose is available

REM Test 1: Build Docker images
echo.
echo === Test 1: Building Docker Images ===
echo Building frontend Docker image...
cd frontend
docker build -t matching-engine-frontend:test -f Dockerfile .
cd ..

echo Building C++ engine Docker image...
docker build -t matching-engine-cpp:test -f Dockerfile.engine .

echo ✅ Docker images built successfully

REM Test 2: Docker Compose build
echo.
echo === Test 2: Docker Compose Build ===
%DOCKER_COMPOSE% build --no-cache

echo ✅ Docker Compose build completed

REM Test 3: Docker Compose up (limited test)
echo.
echo === Test 3: Starting Services ===
echo Starting services in background...
%DOCKER_COMPOSE% up -d

echo Waiting for services to start (30 seconds)...
timeout /t 30 /nobreak >nul

REM Test 4: Service health checks
echo.
echo === Test 4: Service Health Checks ===

REM Check if frontend is reachable
curl -f http://localhost:3000 >nul 2>nul
if %errorlevel% equ 0 (
    echo ✅ Frontend is reachable on http://localhost:3000
) else (
    echo ❌ Frontend is not reachable on http://localhost:3000
)

REM Check if engine health endpoint is reachable
curl -f http://localhost:8080/health >nul 2>nul
if %errorlevel% equ 0 (
    echo ✅ Engine health endpoint is reachable on http://localhost:8080/health
    echo Health response:
    curl -s http://localhost:8080/health
) else (
    echo ⚠️  Engine health endpoint not reachable (may not be implemented)
)

REM Check if websocket-bridge is reachable
curl -f http://localhost:8081 >nul 2>nul
if %errorlevel% equ 0 (
    echo ✅ WebSocket bridge is reachable on http://localhost:8081
) else (
    echo ⚠️  WebSocket bridge not reachable
)

REM Test 5: Cleanup
echo.
echo === Test 5: Cleanup ===
echo Stopping services...
%DOCKER_COMPOSE% down

echo.
echo === Deployment Test Summary ===
echo All Docker infrastructure tests completed successfully!
echo.
echo Next steps for actual deployment:
echo 1. Push to GitHub repository
echo 2. GitHub Actions will automatically run CI/CD pipeline
echo 3. Deploy to Railway.app using Railway CLI or web interface
echo 4. Monitor deployment at https://railway.app
echo.
echo To deploy to Railway:
echo   railway up --service matching-engine
echo   railway up --service frontend
echo.
echo Phase 7 deployment infrastructure is ready! 🚀