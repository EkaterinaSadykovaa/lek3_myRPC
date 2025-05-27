Команды:
make all //Создаёт исполняемые файлы
sudo make install  //Устанавливает скомпилированные файлы в системные директории
make deb  //Создаёт deb-пакеты
sudo systemctl start myRPC-server  //Запускает сервер myRPC как демона (фоновую службу) через Systemd
sudo systemctl enable myRPC-server  //Добавляет сервис myRPC-server в автозагрузку
sudo systemctl status myRPC-server  //Показывает статус сервиса
myRPC-client -c "echo /hi" -h 127.0.0.1 -p 12345 -s  //Запуск клиента
