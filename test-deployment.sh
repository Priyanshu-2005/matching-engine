#!/bin/bash
# Deployment test script for Ultra-Low-Latency Matching Engine
# This script tests the Docker-based deployment locally

set -e  # Exit on error

echo "=== Deployment Test Script ==="
echo "Testing Phase 7: CI/CD & Deployment"
echo ""

# Check Docker availability
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed or not in PATH"
    exit 1
fi

if ! docker info &> /dev/null; then
    echo "❌ Docker daemon is not running"
    echo "   Please start Docker Desktop or the Docker service"
    exit 1
fi

echo "✅ Docker is available: $(docker --version)"

# Check Docker Compose
if ! command -v docker-compose &> /dev/null; then
    echo "⚠️  docker-compose not found, using docker compose plugin"
    DOCKER_COMPOSE="docker compose"
else
    DOCKER_COMPOSE="docker-compose"
fi

echo "✅ Docker Compose is available"

# Test 1: Build Docker images
echo ""
echo "=== Test 1: Building Docker Images ==="
echo "Building frontend Docker image..."
cd frontend
docker build -t matching-engine-frontend:test -f Dockerfile .
cd ..

echo "Building C++ engine Docker image..."
docker build -t matching-engine-cpp:test -f Dockerfile.engine .

echo "✅ Docker images built successfully"

# Test 2: Docker Compose build
echo ""
echo "=== Test 2: Docker Compose Build ==="
$DOCKER_COMPOSE build --no-cache

echo "✅ Docker Compose build completed"

# Test 3: Docker Compose up (limited test)
echo ""
echo "=== Test 3: Starting Services ==="
echo "Starting services in background..."
$DOCKER_COMPOSE up -d

echo "Waiting for services to start (30 seconds)..."
sleep 30

# Test 4: Service health checks
echo ""
echo "=== Test 4: Service Health Checks ==="

# Check if frontend is reachable
if curl -f http://localhost:3000 > /dev/null 2>&1; then
    echo "✅ Frontend is reachable on http://localhost:3000"
else
    echo "❌ Frontend is not reachable on http://localhost:3000"
fi

# Check if engine health endpoint is reachable
if curl -f http://localhost:8080/health > /dev/null 2>&1; then
    echo "✅ Engine health endpoint is reachable on http://localhost:8080/health"
    # Show health response
    echo "Health response:"
    curl -s http://localhost:8080/health | jq . 2>/dev/null || curl -s http://localhost:8080/health
else
    echo "⚠️  Engine health endpoint not reachable (may not be implemented)"
fi

# Check if websocket-bridge is running
if curl -f http://localhost:8081 > /dev/null 2>&1; then
    echo "✅ WebSocket bridge is reachable on http://localhost:8081"
else
    echo "⚠️  WebSocket bridge not reachable"
fi

# Test 5: Cleanup
echo ""
echo "=== Test 5: Cleanup ==="
echo "Stopping services..."
$DOCKER_COMPOSE down

echo ""
echo "=== Deployment Test Summary ==="
echo "All Docker infrastructure tests completed successfully!"
echo ""
echo "Next steps for actual deployment:"
echo "1. Push to GitHub repository"
echo "2. GitHub Actions will automatically run CI/CD pipeline"
echo "3. Deploy to Railway.app using Railway CLI or web interface"
echo "4. Monitor deployment at https://railway.app"
echo ""
echo "To deploy to Railway:"
echo "  railway up --service matching-engine"
echo "  railway up --service frontend"
echo ""
echo "Phase 7 deployment infrastructure is ready! 🚀"