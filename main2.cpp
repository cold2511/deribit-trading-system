#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <mutex>
#include <map>
#include <sstream>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using websocketpp::connection_hdl;
typedef websocketpp::client<websocketpp::config::asio_client> ws_client;

// Utility to measure time durations
class Benchmark {
public:
    void start() { 
        start_time = std::chrono::high_resolution_clock::now(); 
    }
    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    }
    long get_duration() const { return duration; }

private:
    std::chrono::high_resolution_clock::time_point start_time, end_time;
    long duration = 0;
};

// CURL response writer
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

void log(const std::string& message) {
    std::cout << "[LOG] " << message << std::endl;
}




//function to authenticate user id and password which in returns a token which is unique and 
//to be used all other functions
std::string authenticate(const std::string& client_id, const std::string& client_secret) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (curl) {
        std::string postFields = "grant_type=client_credentials&client_id=" + client_id + "&client_secret=" + client_secret;

        curl_easy_setopt(curl, CURLOPT_URL, "https://www.deribit.com/api/v2/public/auth");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            json j = json::parse(readBuffer);
            return j["result"]["access_token"];
        } else {
            std::cerr << "Authentication failed\n";
            return "";
        }
    }
    return "";
}

// Fetch open positions from Deribit
void fetch_open_positions(const std::string& token, const std::string& currency) {
    CURL* curl = curl_easy_init();
    if (!curl) return;

    std::string readBuffer;
    struct curl_slist* headers = nullptr;
    std::string auth_header = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, auth_header.c_str());

    std::string url = "https://www.deribit.com/api/v2/private/get_positions?currency=" + currency;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    std::cout << "[POSITIONS] " << currency << ": " << readBuffer << std::endl;
}
///function to place oreder takes token and currency,price at which to be sold and amout at which to be bought
//the price and amount taken as string values


std::string place_limit_order(const std::string& access_token,const std::string& curr,const std::string& am,const std::string& pr) {
    std::string url = "https://www.deribit.com/api/v2/private/buy";
    std::string readBuffer;
    CURL* curl = curl_easy_init();
    if (curl) {
        // std::vector<std::string> ins={"BTC","ETH"};
        // cout<<"for btc press 1 or for eth press 2"<<endl;
        // int lo;
        // cin>>lo;
        // std::string curr;
        // if(lo>=1 && lo<=2){
        //     curr=ins[lo-1];
        // }
        // else{
        //     curr=ins[0];
        // }
        // std:: string am,pr;
        // cout<<"set amount"<<endl;
        // cin>>am;
        // cout<<"set price"<<endl;
        // cin>>pr;
        std::string postFields = "instrument_name="+curr+"-PERPETUAL&amount="+am+"&price="+pr+"&type=limit";
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

//this is the function which displays the real time updates 
//this function uses websocket for fetching real time data consistently
void on_message(ws_client* c, websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
    json j = json::parse(msg->get_payload());
    if (j.contains("params")) {
        auto method = j["method"].get<std::string>();
        if (method.find("book") != std::string::npos) {
            std::cout << "[ORDERBOOK UPDATE] " << j.dump(2) << std::endl;
        } else if (method.find("trades") != std::string::npos) {
            std::cout << "[TRADE UPDATE] " << j.dump(2) << std::endl;
        }
    }
}


void start_websocket() {
    ws_client client;

    try {
        client.set_access_channels(websocketpp::log::alevel::none);
        client.clear_access_channels(websocketpp::log::alevel::all);
        client.init_asio();
        client.set_message_handler(bind(&on_message, &client, std::placeholders::_1, std::placeholders::_2));

        websocketpp::lib::error_code ec;
        ws_client::connection_ptr con = client.get_connection("wss://www.deribit.com/ws/api/v2", ec);
        if (ec) return;

        client.connect(con);
        std::thread t([&client]() { client.run(); });

        
        std::vector<std::string> instruments = {
            "book.BTC-PERPETUAL.raw",
            "trades.BTC-PERPETUAL.raw",
            "book.ETH-PERPETUAL.raw",
            "trades.ETH-PERPETUAL.raw"
        };

        json subscribe_msg = {
            {"jsonrpc", "2.0"},
            {"id", 42},
            {"method", "public/subscribe"},
            {"params", {{"channels", instruments}}}
        };

        std::string serialized = subscribe_msg.dump();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        con->send(serialized);

        t.join();

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception: " << e.what() << std::endl;
    }
}


//this is the function which calls the place_limit_order function for placing order this function is called 
///inside the trade loop function 

void place_order(const std::string& token,const std::string& curr,const std::string& am,const std::string& pr) {
    
    std::cout << "[ORDER] Placing limit order with amount: " << global_amount << " and price: " << global_price << std::endl;
    std::string result = place_limit_order(token,curr, am, pr);
    std::cout << "[ORDER RESPONSE] " << result << std::endl;


//std::this_thread::sleep_for(std::chrono::milliseconds(25));
}
//this run_strategy which determines the amount at which the trader has to buy the order
//this functions returns the currency,amount and price enterd by the user

std::vector<string> run_strategy(const std::string& token) {
    std:: string curr,am,pr;
    cout<<"enter currency"<<endl;
    cin>>curr;
    cout<<"enter amount"<<endl;
    cin>>am;
    cout<<"enter price"<<endl;
    cin>>pr;
    std::vector<string> ret={curr,am,pr}
    //place_order(token,curr,am,pr);
    //std::this_thread::sleep_for(std::chrono::milliseconds(15));
}



//this is the main trading loop which is called by the int main() function takes token
//as a parameter it also calculates the time durationor the latency which was held during processing and fetching of data and  uying selling the currencies


void trading_loop(const std::string& token) {
    Benchmark loop_benchmark;
    Benchmark fetch_bm, strategy_bm, order_bm;

    loop_benchmark.start();

    // fetch_bm.start();
    // fetch_market_data();
    // fetch_bm.stop();
    std::vector<string> vec;

    strategy_bm.start();
    vec=run_strategy(token);
    
    strategy_bm.stop();

    order_bm.start();
    place_order(token,vec[0],vec[1],vec[2]);
    order_bm.stop();

    loop_benchmark.stop();

    std::cout << "\n=== Latency Report (microseconds) ===" << std::endl;
    std::cout << "Market Data Latency: " << fetch_bm.get_duration() << " us" << std::endl;
    std::cout << "Strategy Latency: " << strategy_bm.get_duration() << " us" << std::endl;
    std::cout << "Order Placement Latency: " << order_bm.get_duration() << " us" << std::endl;
    std::cout << "End-to-End Trading Loop Latency: " << loop_benchmark.get_duration() << " us" << std::endl;
}

int main() {
    
    cout<<"enter client_id"<<endl;
    std::string client_id;
    cin>>client_id;
    std::string client_secret;
    cout<<"Enter client_secret_key"<<endl;
    cin>>client_secret;
    std::string token = authenticate(client_id, client_secret);
    if (token.empty()) {
       std::cerr << "[ERROR] Failed to authenticate. Exiting." << std::endl;
        return 1;
    }

    std::vector<std::string> currencies = {"BTC", "ETH", "SOL"};
    cout<<"for BTC press 1"<<endl;
    cout<<"for ETH press 2"<<endl;
    cout<<"for SOL press 3"<<endl;
    int h;
    cin>>h;
    
    std:: string cur;
    if(h>=1 && h<=3){
        cur=currencies[h-1];

    }
    else{
        cur="BTC";
    }


    fetch_open_positions(token, cur);



    //fetch_open_positions(token);
    std::thread ws_thread(start_websocket);
    trading_loop(token);
    ws_thread.join();

    return 0;
}
