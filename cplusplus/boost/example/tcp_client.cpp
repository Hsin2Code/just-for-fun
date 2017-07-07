#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#include <pthread.h>


#include "boost_async_tcp_client.hpp"

#define UUID "f1a78d51-e964-4c51-8516-4a4354d1c7a9"

#define SERV_HOST "192.168.137.101"
#define SERV_PORT "16016"

using namespace std;

class client : public chat_client
{
private:
    const string host_;
    const string port_;
    bool bLogin;

    pthread_t tid_;              // 线程ID
    static void * thread_proxy_func(void *args) {
        client *c = static_cast<client *>(args);
        c->run();
        return NULL;
    }
public:
    client() : host_(SERV_HOST),
               port_(SERV_PORT),
               bLogin(false) {
        connect(host_, port_);
    }
    ~client() {}
    bool start() {              // 开启通讯线程
        pthread_create(&tid_, NULL, thread_proxy_func, this) == 0;
    }
    void stop() {
        disconnect();
        if(tid_ > 0) {
            pthread_join(tid_, NULL);
        }
    }
    void pull_policy();         // 拉取策略

private:
    void login();               // 登录
    void reconnect() {
        cout << "Re-Connecting ..." << endl;
        bLogin = false;
        connect(host_, port_);
    }
    void handle_response(std::string strJson);
    void do_heartbeat();
};

// 登录
void
client::login()
{
    using namespace rapidjson;
    Document document;
    Document::AllocatorType& allocator = document.GetAllocator(); // 获取空间适配器
    // 将当前的Document设置为一个object，也就是说整个Document是一个object类型的doc元素
    document.SetObject();
    // 添加属性
    document.AddMember("from", "vssa", allocator);
    document.AddMember("res", "conn", allocator);
    document.AddMember("opt", "login", allocator);

    Value locationObj(kObjectType);
    locationObj.AddMember("agentType", "vssa", allocator);
    locationObj.AddMember("agentUuid", UUID, allocator);
    locationObj.AddMember("connUuid", UUID, allocator);
    locationObj.AddMember("version", "", allocator);
    document.AddMember("data", locationObj, allocator);

    /* 生成内容 */
    StringBuffer buffer;
    //PrettyWriter<StringBuffer> writer(buffer);
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);
    string strJson = buffer.GetString();

    chat_message msg;
    msg.body_length(strJson.length());
    memcpy(msg.body(), strJson.data(), msg.body_length());
    msg.encode_header();
    write(msg);
}
// 心跳
void
client::do_heartbeat()
{
    using namespace rapidjson;
    Document document;
    Document::AllocatorType& allocator = document.GetAllocator(); // 获取空间适配器
    // 将当前的Document设置为一个object，也就是说整个Document是一个object类型的doc元素
    document.SetObject();
    // 添加属性
    document.AddMember("from", "vssa", allocator);
    document.AddMember("res", "conn", allocator);
    document.AddMember("opt", "active", allocator);

    Value locationObj(kObjectType);
    locationObj.AddMember("agentType", "vssa", allocator);
    locationObj.AddMember("agentUuid", UUID, allocator);
    locationObj.AddMember("connUuid", UUID, allocator);
    locationObj.AddMember("version", "", allocator);
    document.AddMember("data", locationObj, allocator);

    /* 生成内容 */
    StringBuffer buffer;
    //PrettyWriter<StringBuffer> writer(buffer);
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);
    string strJson = buffer.GetString();

    chat_message msg;
    msg.body_length(strJson.length());
    memcpy(msg.body(), strJson.data(), msg.body_length());
    msg.encode_header();
    write(msg);
    std::cout << "Heartbeat..." <<std::endl;
    heartbeat();
}
void
client::pull_policy()
{
    using namespace rapidjson;
    Document document;
    Document::AllocatorType& allocator = document.GetAllocator(); // 获取空间适配器
    // 将当前的Document设置为一个object，也就是说整个Document是一个object类型的doc元素
    document.SetObject();
    // 添加属性
    document.AddMember("from", "vssa", allocator);
    document.AddMember("res", "policy/object", allocator);
    document.AddMember("opt", "pull", allocator);
    Value nullObject(kNullType);
    document.AddMember("data", nullObject, allocator);

    /* 生成内容 */
    StringBuffer buffer;
    //PrettyWriter<StringBuffer> writer(buffer);
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);
    string strJson = buffer.GetString();

    chat_message msg;
    msg.body_length(strJson.length());
    memcpy(msg.body(), strJson.data(), msg.body_length());
    msg.encode_header();
    write(msg);
}

// 响应
void
client::handle_response(std::string strJson)
{
    cout << "RX: " << strJson << endl;
    using namespace rapidjson;
    Document newDoc;
    newDoc.Parse<0>(strJson.c_str());
    if(newDoc.HasParseError()) {
        cout << "Pares Json Error" << endl;
    }

    if(newDoc.HasParseError()) {
        cout << newDoc.GetParseError() << endl;
    }
    string res, opt, from;
    if(newDoc.HasMember("from")) {
        from = newDoc["from"].GetString();
    }
    if(from != "vsmc") {
        return;
    }

    cout << res << " " << opt << endl;

    if(newDoc.HasMember("res"))
        res = newDoc["res"].GetString();

    if(newDoc.HasMember("opt"))
        opt = newDoc["opt"].GetString();

    bool isLogin = false;
    if(newDoc.HasMember("data")) {
        cout << "yes" << endl;
        Value locationObj(kObjectType);
        if(newDoc["data"].HasMember("isLogin"))
            isLogin = newDoc["data"]["isLogin"].GetBool();
    }

    if(opt == "updateStatus") {
        if(res == "conn") {
            if(isLogin && !bLogin) {
                bLogin = true;
                cout << "开始心跳 " << endl;
                heartbeat();
                pull_policy();
            }
        }
    }
    if(opt == "change") {
        if(res == "policy/object") {
            pull_policy();
        }
    }
}



int main(int argc, char* argv[])
{

    client c;

    c.start();

    cout << "开始通讯线程" << endl;

    sleep(3);

    c.pull_policy();

    sleep(4);

    c.stop();
    cout << "结束通讯线程" << endl;
    return 0;
}
