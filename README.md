# Зайцева Юлия Александровна, БПИ213

# Домашнее задание №2
Применение семафоров для организации взаимодействия параллельных процессов

## Вариант задания 9: 
Задача о супермаркете. В супермаркете работают два кассира, покупатели заходят в супермаркет, делают покупки и становятся в очередь к случайному кассиру. Пока очередь пуста, кассир «спит», как только появляется покупатель, кассир «просыпается». Покупатель «спит» в очереди, пока не подойдет к кассиру. Создать приложение, моделирующее рабочий день супермаркета. Каждого кассира и каждого покупателя реализовать в виде отдельных процессов

## Реализация программы на 4

a. Были созданы несколько процессов: для первого продавца, второго и всех посетителей. Все потетители приходят к продавцам в разное время и встают в очередь. Если она пуста, то они случайным образом выбирают продавца и идут делать покупки. Причем им требуется время на то, чтобы уйти от продавца + продавец будет обслуживать некоторое время. Когда они заканчивают покупки, освобождают очередь. Когда продавец берет заканчивает с покупателем, он сообщает ему об этом. Когда заканчивается рабочий день, один продавец сообщает об этом другому.

b. Было создано 5 семафор стандарта UNIX SYSTEM V: для первого продавца, второго, очереди посетителей, а также для связи продавцов с покупателями. Когда посетитель подходил к продавцу, значение его семафора(1 или 2) увеличивалось. При обслуживании клиента значение семафора (первого или второго) уменьшалось.  Аналогично и с вставанием в очередь. Когда завершалось обслуживание, значение 4 или 5 семафора увеличивалось, и покупатель снова становился активным. Также, для корректной работы программы необходимо хранить кол-во клиентов, которых осталось обслужить. Для этого был создан объект разделяемой памяти.

c. Предусмотрено корректное завершение по прерыванию с клавиатуры по соответствующему сигналу: удаляются семафоры и объект разделяемой памяти.

d. На экран выводятся этапы покупок: когда покупатель подошел на кассу, когда для него освободился кассир, когда началось обслуживание и когда посетитель покинул магазин.

e. Кол-во посетителей вводится с командной строки.

Пример выполнения программы: 

![ФАйл](https://user-images.githubusercontent.com/97798186/233171877-0d504f05-e0d3-4b8d-a183-2f61ab6b2a23.jpg)

## Реализация программы на 5

a. Вместо семафор стандарта UNIX SYSTEM V были использованы именованные POSIX семафоры.

Пример выполнения программы: 

![ФАйл](https://user-images.githubusercontent.com/97798186/233320600-c363f251-44aa-4e91-a6fd-b2936db7eca6.jpg)

## Реализация программы на 6

a. Вместо именованных POSIX семафор были использованы неименованные, расположенные в разделяемой памяти.

Пример выполнения программы: 

![ФАйл](https://user-images.githubusercontent.com/97798186/233328194-4ed6bad4-b98f-4bde-af28-57c0f36ee070.jpg)

## Реализация программы на 7

a. Были использованы именованные POSIX семафоры.

b. Описание программ: 

+ cashier1.c - первый продавец. Он приходит на кассу и ждет, пока магазин откроется. Когда магазин открылся и пришли покупатели, в терминал выводится сообщение по ходу их обслуживания.
+ cashier2.c - второй продавец. Как и первый продавец, он приходит на кассу и ждет, пока магазин откроется. Когда магазин открылся и пришли покупатели, в терминал выводится сообщение по ходу их обслуживания.  
+ cust.c - покупатели. В самом начале магазин открывается (что сообщается через семафор start продавцам) и начинают приходить покупатели. В терминал выводятся сообщения о том, что они встали в очередь, подошли к продавцу, вышли из магазина.

с. Запуск программ: сначала необходимо запустить cashier1.c и cashier2.c, а потом cust.c. Продавцы приходят в магазин раньше, чем он открывается. Количество посетителей нужно ввести только один раз: при запуске cust.c.

Пример работы программы:

![ФАйл](https://user-images.githubusercontent.com/97798186/233357202-e47019de-0c7a-4899-bd43-8892900d34a5.jpg)

## Реализация программы на 8

Были использованы семафоры стандарта UNIX SYSTEM V вместо семафор POSIX.

Пример работы программы:

![ФАйл](https://user-images.githubusercontent.com/97798186/233445540-8bdd0bfa-5e9c-41bb-994e-e92a917e54a1.jpg)

