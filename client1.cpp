#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

std::string xor_encrypt_decrypt(const std::string &input, const std::string &key) {
    std::string output = input;
    size_t key_len = key.length();
    for (size_t i = 0; i < input.length(); ++i) {
        output[i] = input[i] ^ key[i % key_len];
    }
    return output;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    std::string key;

    // Подключаемся к серверу
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Запрашиваем ключ шифрования
    std::cout << "Enter encryption key: ";
    std::cin >> key;

    if (key.empty()) {
        std::cerr << "Key cannot be empty.\n";
        close(sock);
        return -1;
    }

    // Цикл общения
    while (true) {
        std::string message;
        std::cout << "Enter message: ";
        std::cin.ignore();  // Игнорируем лишний символ новой строки
        std::getline(std::cin, message);

        // Шифруем сообщение
        std::string encrypted_message = xor_encrypt_decrypt(message, key);

        // Отправляем зашифрованное сообщение на сервер
        send(sock, encrypted_message.c_str(), encrypted_message.size(), 0);

        // Получаем зашифрованный ответ от сервера
        int valread = read(sock, buffer, sizeof(buffer));
        if (valread <= 0) {
            std::cerr << "Connection lost.\n";
            break;
        }

        // Расшифровываем ответ
        std::string decrypted_message = xor_encrypt_decrypt(std::string(buffer, valread), key);
        std::cout << "Received message: " << decrypted_message << "\n";
    }

    close(sock);
    return 0;
}