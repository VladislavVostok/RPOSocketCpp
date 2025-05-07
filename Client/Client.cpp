#include <iostream>
#include "product.h"
#include "cross_platform_socket.h"
#include <vector>
#include <sstream>


vector<string> split(const string&, char);


int main()
{
	setlocale(LC_ALL, "");

	try {
		init_network();
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		check_socket_error(sock, "Ошибка создания сокета!");

		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(5555);

		if (inet_pton(AF_INET, "45.139.78.128", &serverAddr.sin_addr) <= 0) {
		//if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
			throw runtime_error("Неверный адрес сервера");
		}

		check_socket_error(
			connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)),
			"Ошибка подключения к серверу"
		);
		
		cout << "Подключение к серверу успешное!" << endl;

		while (true) {
			cout << "\nМеню:" << endl;
			cout << "1. Получить список всех товаров." << endl;
			cout << "2. Получить информацию от товаре по ID" << endl;
			cout << "3. Добавление нового товара (с указанием ID)" << endl;
			cout << "4. Добавление нового товара (авто-генерацией ID)" << endl;
			cout << "5. Выход" << endl;

			int choice;
			cin >> choice;
			cin.ignore();

			string request;

			if (choice == 1) {
				request = "LIST_ALL";
			}
			else if (choice == 2) {
				cout << "Введите ID товара: ";
				int id;
				cin >> id;
				request = "GET " + to_string(id);
			}
			else if (choice == 3) {
				Product p = Product::input_product(true);
				request = "ADD " + p.to_string_custom();
			}
			else if (choice == 4) {
				Product p = Product::input_product(false);
				request = "ADD_AUTOID " + p.to_string_custom();
			}
			else if (choice == 5) {
				request = "EXIT";
			}
			else {
				cout << "Неверный выбор!" << endl;
				continue;
			}
		

			int byteSent = send(sock, request.c_str(), request.size(), MSG_NOSIGNAL);
			check_socket_error(byteSent, "Ошибка отправки запроса!");

			char buffer[1024] = { 0 };
			int byteRead = recv(sock, buffer, sizeof(buffer), 0);
			check_socket_error(byteRead, "Ошибка получения ответа!");

			if (byteRead <= 0) {
				throw runtime_error("Сервер закрыл соединение!");
			}

			string response(buffer, byteRead);

			// Обработка ответа

			if (response.rfind("ERROR|", 0) == 0) {
				cerr << "Ошибка: " << response.substr(6) << endl;
			}
			else if (response.rfind("OK|", 0) == 0) {
				Product p = Product::from_string(response.substr(3));
				p.print();
			}
			else if (response == "EXITED") {
				cout << "Всего хорошого!";
				break;
			}
			else {
				vector<string> products = split(response, '\n');
				for (const auto& productStr : products) {
					if (!productStr.empty()) {
						Product p = Product::from_string(productStr);
						p.print();
						cout << "_________________________" << endl;
					}
				}
			}
		}

		close_socket(sock);
		cleanup_network();

	}
	catch (const exception& e) {
		cerr << "Ошибка: " << e.what() << endl;
		cleanup_network();
		return 1;
	}
	return 0;
}

vector<string> split(const string& s, char delimiter) {
	vector<string> tokens;
	string token;
	istringstream tokenStream(s);

	while (getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}

	return tokens;
}
