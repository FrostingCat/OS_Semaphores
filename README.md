# Зайцева Юлия Александровна, БПИ213

# Домашнее задание №2
Применение семафоров для организации взаимодействия параллельных процессов

## Вариант задания 9: 
Задача о супермаркете. В супермаркете работают два кассира, покупатели заходят в супермаркет, делают покупки и становятся в очередь к случайному кассиру. Пока очередь пуста, кассир «спит», как только появляется покупатель, кассир «просыпается». Покупатель «спит» в очереди, пока не подойдет к кассиру. Создать приложение, моделирующее рабочий день супермаркета. Каждого кассира и каждого покупателя реализовать в виде отдельных процессов

## Реализация программы на 4

a. Были созданы несколько процессов: для первого продавца, второго и всех посетителей. Все потетители приходят к продавцам в разное время и встают в очередь. Если она пуста, то они случайным образом выбирают продавца и идут делать покупки. Причем им требуется время на то, чтобы подойти к продавцу и уйти от него + продавец будет обслуживать некоторое время. Когда они заканчивают покупки, освобождают очередь. Продавец, в свою очередь, некоторое время готовится к следующему покупателю.

b. Было создано три семафора стандарта UNIX SYSTEM V: для первого продавца, второго и очереди посетителей. При обслуживании клиента значение семафора (первого или второго) уменьшалось. А когда посетитель обслужен - увеличивалось. Аналогично и с вставанием в очередь. Также, для корректной работы программы необходимо хранить кол-во клиентов, которых осталось обслужить. Для этого был создан объект разделяемой памяти.

c. Предусмотрено корректное завершение по прерыванию с клавиатуры по соответствующему сигналу: удаляются семафоры и объект разделяемой памяти.

d. На экран выводятся этапы покупок: когда покупатель подошел на кассу, когда для него освободился кассир, когда началось обслуживание и когда посетитель покинул магазин.

e. Кол-во посетителей вводится с командной строки.

Пример выполнения программы: 

![ФАйл](https://user-images.githubusercontent.com/97798186/233171877-0d504f05-e0d3-4b8d-a183-2f61ab6b2a23.jpg)

