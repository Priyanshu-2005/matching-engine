const WebSocket = require('ws');
const http = require('http');

// Create HTTP server for health checks
const server = http.createServer((req, res) => {
    if (req.url === '/health') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ status: 'ok', clients: wsServer.clients.size }));
    } else {
        res.writeHead(404);
        res.end();
    }
});

// Create WebSocket server
const wsServer = new WebSocket.Server({ server, path: '/ws' });

// Mock data generators
function generateTradeEvent() {
    return {
        type: 'trade',
        trade_id: Math.floor(Math.random() * 1000000),
        maker_order_id: Math.floor(Math.random() * 10000),
        taker_order_id: Math.floor(Math.random() * 10000),
        price: 15000 + Math.floor(Math.random() * 500) - 250,
        quantity: Math.floor(Math.random() * 100) + 1,
        timestamp: Date.now(),
        side: Math.random() > 0.5 ? 'Buy' : 'Sell'
    };
}

function generateOrderBook() {
    const bids = [];
    const asks = [];

    for (let i = 0; i < 10; i++) {
        bids.push({
            price: 14950 + i * 10,
            quantity: Math.floor(Math.random() * 1000) + 100
        });
        asks.push({
            price: 15050 + i * 10,
            quantity: Math.floor(Math.random() * 1000) + 100
        });
    }

    return {
        type: 'orderbook',
        bids: bids.sort((a, b) => b.price - a.price),
        asks: asks.sort((a, b) => a.price - b.price),
        bestBid: 14990,
        bestAsk: 15010,
        spread: 20
    };
}

function generateLatencyMetrics() {
    return {
        type: 'latency',
        current: Math.floor(Math.random() * 200) + 50,
        average: 142,
        p50: 142,
        p90: 287,
        p99: 512,
        p999: 890,
        max: 1200,
        min: 45
    };
}

// Client management
const clients = new Set();

wsServer.on('connection', (ws) => {
    console.log('New WebSocket client connected');
    clients.add(ws);

    // Send initial data
    ws.send(JSON.stringify(generateOrderBook()));
    ws.send(JSON.stringify(generateLatencyMetrics()));

    ws.on('close', () => {
        console.log('Client disconnected');
        clients.delete(ws);
    });

    ws.on('error', (err) => {
        console.error('WebSocket error:', err);
    });
});

// Broadcast mock data at intervals
setInterval(() => {
    const tradeEvent = generateTradeEvent();
    // Wrap in trade_batch format expected by frontend
    const tradeBatch = {
        type: 'trade_batch',
        data: {
            events: [tradeEvent]
        }
    };
    const message = JSON.stringify(tradeBatch);

    clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(message);
        }
    });

    // Update order book every 5 seconds
    if (Date.now() % 5000 < 100) {
        const orderBook = generateOrderBook();
        const orderBookMessage = JSON.stringify(orderBook);
        clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(orderBookMessage);
            }
        });
    }

    // Update latency every 2 seconds
    if (Date.now() % 2000 < 100) {
        const latency = generateLatencyMetrics();
        const latencyMessage = JSON.stringify(latency);
        clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(latencyMessage);
            }
        });
    }
}, 100); // 10 Hz update rate

// Start server
const PORT = 8080;
server.listen(PORT, () => {
    console.log(`WebSocket bridge server running on port ${PORT}`);
    console.log(`WebSocket endpoint: ws://localhost:${PORT}/ws`);
    console.log(`Health check: http://localhost:${PORT}/health`);
    console.log('\n=== Simulating Ultra-Low-Latency Matching Engine ===');
    console.log('• Mock trade events: 10 Hz');
    console.log('• Order book updates: 0.2 Hz');
    console.log('• Latency metrics: 0.5 Hz');
    console.log('• Target latency: < 200 ns (simulated)');
});