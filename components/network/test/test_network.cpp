#include <gtest/gtest.h>
#include <interface/network-i.h>
#include <helper/obj-ref-impl.hpp>
#include <thread>
#include <chrono>

#include <com-loader.hpp>

using namespace SOUI;

class NetworkTest : public ::testing::Test {
    SComLoader m_comLoader;
protected:
    void SetUp() override {
        // Initialize network components
        m_comLoader.CreateInstance(_T("network"), (IObjRef**)&m_network);
        ASSERT_TRUE(m_network != nullptr);
    }

    void TearDown() override {
        m_network = nullptr;
    }

    SAutoRefPtr<INetwork> m_network;
};

// Test UDP communication
TEST_F(NetworkTest, UdpCommunication) {
    // Create UDP server socket
    SAutoRefPtr<IUdpSocket> serverSocket;
    serverSocket = m_network->CreateUdpSocket();
    ASSERT_TRUE(serverSocket != nullptr);

    // Create UDP client socket
    SAutoRefPtr<IUdpSocket> clientSocket;
    clientSocket = m_network->CreateUdpSocket();
    ASSERT_TRUE(clientSocket != nullptr);

    // Bind server socket
    ASSERT_TRUE(serverSocket->Create("127.0.0.1", 8888));
    ASSERT_TRUE(clientSocket->Create("127.0.0.1", 8889));

    // Send data from client to server
    const char* sendData = "Hello UDP!";
    int sentLen = 0;
    ASSERT_TRUE(clientSocket->SendTo("127.0.0.1", 8888, sendData, strlen(sendData), &sentLen));
    ASSERT_EQ(sentLen, strlen(sendData));

    // Receive data on server
    char recvBuf[1024] = {0};
    unsigned int recvLen = sizeof(recvBuf) - 1;
    char remoteIp[16] = {0};
    uint16_t remotePort = 0;
    
    // Wait a moment for data to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(serverSocket->RecvFrom(remoteIp, remotePort, recvBuf, recvLen));
    ASSERT_EQ(remotePort, 8889);
    ASSERT_EQ(strcmp(recvBuf, sendData), 0);

    // Send response back to client
    const char* response = "Hello back!";
    ASSERT_TRUE(serverSocket->SendTo(remoteIp, remotePort, response, strlen(response), &sentLen));
    ASSERT_EQ(sentLen, strlen(response));

    // Receive response on client
    recvLen = sizeof(recvBuf) - 1;
    memset(recvBuf, 0, sizeof(recvBuf));
    
    // Wait a moment for data to arrive
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(clientSocket->RecvFrom(remoteIp, remotePort, recvBuf, recvLen));
    ASSERT_EQ(strcmp(recvBuf, response), 0);
    ASSERT_EQ(remotePort, 8888);

    // Close sockets
    serverSocket->Close();
    clientSocket->Close();
}

// TCP server thread function
void tcpServerThread(bool& serverReady, bool& testDone) {
    SComLoader comLoader;
    SAutoRefPtr<INetwork> network;
    comLoader.CreateInstance(_T("network"), (IObjRef**)&network);
    
    // Create TCP server socket
    SAutoRefPtr<ITcpServerSocket> serverSocket;
    serverSocket = network->CreateTcpServerSocket();
    
    // Bind server socket
    serverSocket->Create("127.0.0.1", 9999);
    
    // Start listening
    serverSocket->Listen(5);
    
    // Notify server is ready
    serverReady = true;
    
    // Accept client connection
    SAutoRefPtr<ITcpClientSocket> clientSocket;
    clientSocket = serverSocket->Accept();
    
    if (clientSocket != nullptr) {
        // Receive data from client
        char recvBuf[1024] = {0};
        unsigned int recvLen = sizeof(recvBuf) - 1;
        clientSocket->Recv(recvBuf, recvLen);
        
        // Send response back
        const char* response = "Hello TCP!";
        clientSocket->Send(response, strlen(response), nullptr);
        
        // Close client socket
        clientSocket->Close();
    }
    
    // Close server socket
    serverSocket->Close();
    
    // Notify test is done
    testDone = true;
}

// Test TCP communication
TEST_F(NetworkTest, TcpCommunication) {
    bool serverReady = false;
    bool testDone = false;
    
    // Start TCP server thread
    std::thread serverThread(tcpServerThread, std::ref(serverReady), std::ref(testDone));
    
    // Wait for server to be ready
    while (!serverReady) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Create TCP client socket
    SAutoRefPtr<ITcpClientSocket> clientSocket;
    clientSocket = m_network->CreateTcpClientSocket();
    ASSERT_TRUE(clientSocket != nullptr);
    
    // Connect to server
    ASSERT_TRUE(clientSocket->CreateIpV4());
    ASSERT_TRUE(clientSocket->Connect("127.0.0.1", 9999));
    
    // Send data to server
    const char* sendData = "Hello TCP!";
    ASSERT_TRUE(clientSocket->Send(sendData, strlen(sendData), nullptr));
    
    // Receive response from server
    char recvBuf[1024] = {0};
    unsigned int recvLen = sizeof(recvBuf) - 1;
    ASSERT_TRUE(clientSocket->Recv(recvBuf, recvLen));
    ASSERT_EQ(strcmp(recvBuf, "Hello TCP!"), 0);
    
    // Close client socket
    clientSocket->Close();
    
    // Wait for server thread to complete
    serverThread.join();
    ASSERT_TRUE(testDone);
}

// Test TCP client socket creation
TEST_F(NetworkTest, CreateTcpClientSocket) {
    SAutoRefPtr<ITcpClientSocket> clientSocket;
    clientSocket = m_network->CreateTcpClientSocket();
    ASSERT_TRUE(clientSocket != nullptr);
    
    // Test CreateIpV4 method
    ASSERT_TRUE(clientSocket->CreateIpV4());
    
    // Test SetBlocking method
    ASSERT_EQ(clientSocket->SetBlocking(true), 0);
    
    // Test IsConnected method (should be false initially)
    ASSERT_FALSE(clientSocket->IsConnected());
    
    // Close socket
    clientSocket->Close();
}

// Test TCP server socket creation
TEST_F(NetworkTest, CreateTcpServerSocket) {
    SAutoRefPtr<ITcpServerSocket> serverSocket;
    serverSocket = m_network->CreateTcpServerSocket();
    ASSERT_TRUE(serverSocket != nullptr);
    
    // Test Create method
    ASSERT_TRUE(serverSocket->Create("127.0.0.1", 7777));
    
    // Test Listen method
    ASSERT_TRUE(serverSocket->Listen(5));
    
    // Test GetClientSocketCount method (should be 0 initially)
    ASSERT_EQ(serverSocket->GetClientSocketCount(), 0);
    
    // Close socket
    serverSocket->Close();
}

// Test UDP socket creation
TEST_F(NetworkTest, CreateUdpSocket) {
    SAutoRefPtr<IUdpSocket> udpSocket;
    udpSocket = m_network->CreateUdpSocket();
    ASSERT_TRUE(udpSocket != nullptr);
    
    // Test Create method
    ASSERT_TRUE(udpSocket->Create("127.0.0.1", 6666));
    
    // Test SetBlocking method
    ASSERT_EQ(udpSocket->SetBlocking(true), 0);
    
    // Test SetBufLength method
    ASSERT_EQ(udpSocket->SetBufLength(8192), 0);
    
    // Close socket
    udpSocket->Close();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
