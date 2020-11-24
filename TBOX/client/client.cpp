#include "client.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <chrono>

#define CHK_RET(ret) if (ret < 0) { return false; }
#define CHK_BOOL(ret) if (ret != true) { return false; }
#define CHK_NULL(ret) if (ret == nullptr) { return false; }
#define CHK_SSL(err) if ((err) == -1) { ERR_print_errors_fp(stderr); return false; }

client::client(std::string ip, int port)
    : ip_(ip), port_(port), fd_(-1), ctx_(nullptr), ssl_(nullptr),
    type_(CLIENT), enable_ssl_(false)
{
}

client::client(std::string ip, int port, size_t fd)
    : ip_(ip), port_(port), fd_(fd), ctx_(nullptr), ssl_(nullptr),
    type_(SERVER), enable_ssl_(false)
{
}

client::client(std::string ip, int port, ssl_tls ssl)
    : ip_(ip), port_(port), fd_(-1), ssl_tls_(ssl), 
    ctx_(nullptr), ssl_(nullptr), type_(CLIENT), enable_ssl_(true)
{
}

client::client(std::string ip, int port, size_t fd, ssl_tls ssl)
    : ip_(ip), port_(port), fd_(fd), ssl_tls_(ssl),
    ctx_(nullptr), ssl_(nullptr), type_(SERVER), enable_ssl_(true)
{
}

client::~client()
{
    std::cout << "~client()" << '\n';
    destroy();
}

bool client::initial()
{
    signal(SIGPIPE, SIG_IGN);

    if (type_ == CLIENT)
    {
        fd_ = create_fd();
        CHK_RET(fd_);

        int ret = connect();
        CHK_RET(ret);
    }

    if (enable_ssl_)
    {
        bool ret = init_ssl();
	std::cout<<"ssl ret init with "<<ret<<'\n';
        CHK_BOOL(ret);
        
    }

    return true;
}

bool client::init_ssl()
{
    // 1.初始化SSL
    // SSL 库初始化
    SSL_library_init();
    // 载入所有 SSL 算法 OpenSSL_add_ssl_algorithms() 和 OpenSSL_add_all_algorithms() 好像都可以用
    OpenSSL_add_all_algorithms();
    // 载入所有 SSL 错误消息
    SSL_load_error_strings();
    // 采用什么协议(SSLv2/SSLv3/TLSv1)
    ctx_ = SSL_CTX_new(ssl_tls_.meth);
    CHK_NULL(ctx_);

    // 2.设置验证
    int ret_b = set_verification(ssl_tls_.two_way_auth);
    CHK_RET(ret_b);

    // 3.加载证书
    ret_b = load_certificate(ssl_tls_.certf.data(), ssl_tls_.private_key.data());
    CHK_BOOL(ret_b);

    // 4.使用ssl管理链接
    ret_b = create_connect();
    CHK_BOOL(ret_b);

    return true;
}

void client::destroy()
{
    if (enable_ssl_)
    {
        // 关闭SSL连接
        SSL_shutdown(ssl_);
        // 释放SSL
        SSL_free(ssl_);
        // 释放CTX
        SSL_CTX_free(ctx_);
    }
    // 关闭socket连接
    close(fd_);
}

bool client::start()
{        
    if (!initial())
    {
        std::cout << "client initial error!" << '\n';
        return false;
    }

    auto self = shared_from_this();
    auto t = std::thread([this, self]()
             {
                work();
             });

    t_.swap(t);
    t_.detach();

    return true;
}

void client::work()
{
    char buff[1024] = {0};
    while (true)
    {
        int ret = recv_msg(buff, 1024);
        if (ret == 0)
        {
            std::cout << "client close socket!" << '\n';
            break;
        }

        if (ret < 0)
        {
            std::cout << "recv_msg error close socket!" << '\n';
            break;
        }

        buff[ret] = '\0';
        ret = send_msg(buff, ret + 1);
        if (ret < 0)
        {
            std::cout << "send_msg error!" << '\n';
        }
        else
        {
            std::cout << "\033[33mclient fd: " << fd_ << ", recv msg: " << buff << "\033[0m" << '\n';
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int client::create_fd()
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        std::cout<<"create fd error"<<'\n';
    }

    return fd;
}

int client::connect()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_.data());
    addr.sin_port = htons(port_);

    socklen_t len = sizeof(addr);

    int ret = ::connect(fd_, (struct sockaddr*)&addr, len);
    if (ret < 0)
    {
        std::cout<<"can't connect"<<'\n';
    }

    return ret;
}

int client::send_msg(const char* msg, int length)
{
    if (enable_ssl_)
    {
        int ret = send_msg_ssl(msg, length);
        return ret;
    }

    int ret = send(fd_, msg, length, 0);
    if (ret < 0)
    {
        // error log
    }

    return ret;
}

int client::recv_msg(char* buffer, int  length)
{
	int ret;
    if (enable_ssl_)
    {
        ret = recv_msg_ssl(buffer, length);
      
    }
	else
	{

	    ret = recv(fd_, buffer, length, 0);
	    if (ret < 0)
	    {
	        // error log
	    }

	    
	}
	return ret;
}

int client::send_msg_ssl(const char* msg, int length)
{
    int len = SSL_write(ssl_, msg, length);
    if (len < 0)
    {
        // error log
    }

    return len;
}

int client::recv_msg_ssl(char* buffer, int length)
{
    int len = SSL_read(ssl_, buffer, length);
    if (len < 0)
    {
        // error log
    }

    return len;
}

bool client::set_verification(bool two_way_auth)
{
    // 双向验证
    // SSL_VERIFY_PEER---要求对证书进行认证，没有证书也会放行
    // SSL_VERIFY_FAIL_IF_NO_PEER_CERT---要求客户端需要提供证书，但验证发现单独使用没有证书也会放行
    // 调用SSL_CTX_set_verify()函数者对证书进行认证，不调用则不认证
    size_t type = SSL_VERIFY_PEER;
    if (two_way_auth)
    {
        type |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    }

    SSL_CTX_set_verify(ctx_, type, NULL);

    // 设置信任根证书
    int ret = SSL_CTX_load_verify_locations(ctx_, ssl_tls_.ca_certf.data(), NULL);
    CHK_SSL(ret);

    // 设置密码
    if (!ssl_tls_.passwd.empty())
    {
        SSL_CTX_set_default_passwd_cb_userdata(ctx_, (void*)ssl_tls_.passwd.data());
    }

    return true;
}

bool client::load_certificate(const char* certf, const char* key)
{
    // 载入数字证书,证书里包含有公钥, 此证书用来发送给对端
    int ret = SSL_CTX_use_certificate_file(ctx_, certf, SSL_FILETYPE_PEM);
    CHK_SSL(ret);

    // 载入用户私钥
    ret = SSL_CTX_use_PrivateKey_file(ctx_, key, SSL_FILETYPE_PEM);
    CHK_SSL(ret);

    // 检查用户私钥是否正确
    ret = SSL_CTX_check_private_key(ctx_);
    CHK_SSL(ret);

    // SSL_CTX_set_cipher_list(ctx_,"RC4-MD5");

    return true;
}

bool client::create_connect()
{
    // 基于 ctx 产生一个新的 SSL
    ssl_ = SSL_new(ctx_);
    CHK_NULL(ssl_);

    // 将连接用户的 socket 加入到 SSL
    SSL_set_fd (ssl_, fd_);

    // 建立 SSL 连接
    int ret = 0;
    if (type_ == SERVER)
    {
        ret = SSL_accept(ssl_);
    }
    else if (type_ == CLIENT)
    {
        ret = SSL_connect(ssl_);
    }
    CHK_RET(ret);

    show_certs(ssl_);

    return true;
}

void client::show_certs(SSL* ssl)
{
    char *line;

    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) 
    {
        if (SSL_get_verify_result(ssl) == X509_V_OK)
        {
            std::cout << "证书验证通过!" << '\n';
        }
        else
        {
            std::cout << "证书验证未通过!" << '\n';
        }

        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        std::cout << "数字证书信息: " << line << '\n';
        free(line);

        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        std::cout << "颁发者: " << line << '\n';
        free(line);
        X509_free(cert);
    } 
    else
    {
        std::cout << "对等方未提供证书!" << '\n';
    }
}


