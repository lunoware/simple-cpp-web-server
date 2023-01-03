#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <string>
#include <fstream>


struct fileData {
  std::string status;
  std::string html;
};

fileData getServerFile(std::string fileName){
    fileData fD;
    
    std::string fileContents = "";
    
    const char *fN = fileName.c_str();
    
    std::ifstream inFile (fN);
    std::cout << "Filename: " << fileName << std::endl;
    
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile,line)) {
            fileContents += line;
        }
        
        fD.status = "200";
        fD.html = fileContents;
    }else{
        fD.status = "404";
        fD.html = "File not found";
    }
    
    inFile.close();
    
    return fD;
}

void acceptConnection(int sockfd, sockaddr_in sockaddr){


    auto addrlen = sizeof(sockaddr);
    int connection;
    while (connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen)) {
    
    
        //int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
        if (connection < 0) {
            std::cout << "Failed to grab connection. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }


        // Read from the connection
        char buffer[100];
        auto bytesRead = read(connection, buffer, 100);

        std::string bufferStr = std::string(buffer);
        std::cout << "The message was: " << bufferStr;
        
        int start = bufferStr.find("/");
        int end = bufferStr.find(" HTTP/1.1");
        std::string fileName = bufferStr.substr(start+1,end-start-1);
        

        fileData fileData = getServerFile(fileName);
        
        std::cout << "HTML: " << fileData.html << fileData.status << std::endl;

        std::string header = "HTTP/1.0 " + fileData.status + " OK\rnContent-Type: text/html\r\nContent-Length: ";
        int htmlLength = fileData.html.length();
        header += std::to_string(htmlLength);
        header += "\r\n\r\n";

        std::string response = header + fileData.html;

        send(connection, response.c_str(), strlen(response.c_str()), 0);

        //acceptConnection(sockfd, sockaddr);
    }
}

int main() {
    // Create a socket (IPv4, TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Listen to port 9999 on any address
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(9999); // htons is necessary to convert a number to
                                   // network byte order
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start listening. Hold at most 10 connections in the queue
    if (listen(sockfd, 10) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    
    // Grab a connection from the queue
    acceptConnection(sockfd, sockaddr);

    // Close the connections
    //  close(connection);
    //  close(sockfd);
}
