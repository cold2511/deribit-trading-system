# 🚀 Deribit Trading System

A high-performance C++-based order execution and management system for cryptocurrency trading on [Deribit](https://www.deribit.com/).

## 📌 Features

- ✅ **Authentication** via client credentials to get access tokens
- 📈 **Live market data streaming** using WebSocket (Order Book & Trades)
- 📤 **Place Limit Orders** via REST API
- 📍 **Fetch Open Positions**
- 📊 **Latency Benchmarking** (strategy logic, API execution, total loop)
- ⚙️ **Modular Design** for easy expansion of trading strategies

## 🛠️ Tech Stack

- C++17
- [libcurl](https://curl.se/libcurl/) – for REST API communication
- [WebSocket++](https://github.com/zaphoyd/websocketpp) – for real-time data
- [nlohmann/json](https://github.com/nlohmann/json) – for JSON parsing

## ⚙️ How to Run

### 🧱 Prerequisites
- C++ compiler (e.g., `g++`, `clang++`)
- `libcurl` and `WebSocket++` installed
- Internet connection (for Deribit API)

### 🧪 Build & Run

```bash
g++ -std=c++17 -lcurl -lboost_system -lpthread -o trading main2.cpp
./trading




📦 Requirements
C++17 or later

libcurl (for REST API)

WebSocket++ (for real-time WebSocket connections)

nlohmann/json (for JSON parsing)


🛠️ Installation & Build Instructions
🐧 For Linux:
sudo apt update
sudo apt install g++ libcurl4-openssl-dev libboost-all-dev

🔨 Build:
g++ main2.cpp -o deribit_trader -lcurl -lboost_system -pthread


▶️ Running the Program
./deribit_trader
You’ll be prompted to:

Enter your client_id and client_secret (Deribit credentials)

Select a currency (BTC, ETH, SOL)

Enter the amount and price for placing the limit order

📡 WebSocket Subscriptions
The application subscribes to these Deribit channels:

book.BTC-PERPETUAL.raw

trades.BTC-PERPETUAL.raw

book.ETH-PERPETUAL.raw

trades.ETH-PERPETUAL.raw

These channels provide live updates on order book and trade data.


⏱️ Performance Benchmarking
After each trading loop, the following latencies (in microseconds) are printed:

Strategy Execution Time

Order Placement Time

End-to-End Loop Latency

This helps analyze how the system performs under different market conditions.


🧠 Strategy Function
The run_strategy() function is a placeholder where you can implement your own logic based on market data and risk analysis.

You can modify this to include:

Indicators (e.g., RSI, Moving Averages)

Custom signals

Risk Management logic

✅ Optimization Techniques
Area	Technique Used
Memory Management:	Efficient use of stack memory, RAII, proper cleanup via curl_easy_cleanup()
Network Communication:	Minimal header usage, persistent connections, optimized POST fields
Data Structure Selection:	std::vector, std::string, unordered_map where needed
Thread Management:	Separate thread for WebSocket to avoid blocking main loop
CPU Optimization:	Lightweight functions, avoids unnecessary recomputations

🤝 Contributing
Feel free to fork this repo, suggest improvements, or submit pull requests!
