#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 2

std::mutex mtx;

void handle_client(int client_socket, int other_client_socket) {
    char buffer[1024];
    while (true) {
        bzero(buffer, 1024);
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break;
        }
        
        // Отправляем зашифрованное сообщение второму клиенту
        mtx.lock();
        write(other_client_socket, buffer, bytes_read);
        mtx.unlock();
    }
    close(client_socket);
}

int main() {
    int server_fd, new_socket, client1_socket, client2_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Создаем сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Привязываем сокет к порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Начинаем слушать соединения
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server is waiting for clients...\n";
    
    // Принимаем подключения от двух клиентов
    client1_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (client1_socket < 0) {
        perror("Client 1 failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Client 1 connected.\n";
    
    client2_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (client2_socket < 0) {
        perror("Client 2 failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Client 2 connected.\n";
    
    // Сервер больше не принимает новых клиентов
    std::cout << "Both clients connected. Starting communication...\n";
    
    // Создаем потоки для обработки клиентов
    std::thread t1(handle_client, client1_socket, client2_socket);
    std::thread t2(handle_client, client2_socket, client1_socket);
    
    t1.join();
    t2.join();
    
    close(server_fd);
    return 0;
}