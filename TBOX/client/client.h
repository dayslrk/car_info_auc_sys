#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <string>
#include <thread>
#include <memory>

#include "openssl/ssl.h"
#include "openssl/err.h"

struct ssl_tls
{
    bool two_way_auth;       // 设置双向认证
    std::string ca_certf;    // CA证书
    std::string certf;       // server/client 证书
    std::string private_key; // server/client 私钥
    std::string passwd;      // 密码
    const SSL_METHOD* meth;  // 协议类型(TLSv1/SSLv2/SSLv3)

    ssl_tls()
    {
        two_way_auth = false;
        ca_certf     = "";
        certf        = "";
        private_key  = "";
        passwd       = "";
        meth         = nullptr;
    }
};

class client : public std::enable_shared_from_this<client>
{
    enum type
    {
        SERVER = 0,
        CLIENT
    };
    public:
        client(std::string ip, int port);
        client(std::string ip, int port, size_t fd);

        // openssl
        client(std::string ip, int port, ssl_tls ssl);
        client(std::string ip, int port, size_t fd, ssl_tls ssl);

        ~client();

        bool initial();
        void destroy();

        bool start();

        int send_msg(const char* msg, int length);
        int recv_msg(char* buffer, int length);
        // openssl
        int send_msg_ssl(const char* msg, int length);
        int recv_msg_ssl(char* buffer, int length);

        size_t get_socket_fd() const { return fd_; }

    private:
        void work();

        int create_fd();
        int connect();

        // openssl
        bool init_ssl(); 
        bool set_verification(bool two_way_auth = false);
        bool load_certificate(const char* certf, const char* key);
        bool create_connect();                                
        void show_certs(SSL* ssl);
    private:
        std::string ip_;
        int         port_;
        size_t      fd_;

        // openssl
        ssl_tls     ssl_tls_;
        SSL_CTX *   ctx_;
        SSL *       ssl_;

        type        type_;
        bool        enable_ssl_;
        std::thread t_;

};

#endif // __CLIENT_H__
