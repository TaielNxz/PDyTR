Programaci ́on Distribuida y Tiempo Real
Pr ́actica 2
1) Buscar y utilizar en vagrant una versi ́on m ́as reciente de Ubuntu LTS que
la dada en la explicaci ́on de pr ́actica. Identificar versi ́on, origen y si tuviera
diferencias, explicar brevemente. Mostrar con una captura de pantalla la
identificaci ́on en VirtualBox la m ́aquina virtual y el inicio de la terminal en
funcionamiento. Consulte a un asistente de IA (deje constancia de cu ́al) si
hay alguna versi ́on en particular que puede ser mejor que otra y por qu ́e.
2) Ejecutar los experimentos del ejercicio 4 a), b) y c) de la pr ́actica anterior
en dos computadoras f ́ısicamente diferentes. Se copia aqu ́ı el enunciado para
mayor comodidad: comunicar 101, 102, 103, 104, 105 y 106 bytes desarrollan-
do experimentos que muestre el tiempo que toma en el cliente
a) La funci ́on write(...) para cada cantidad de datos. Tomando C como
ejemplo:
t0 = ...
write(...)
t1 = ...
tiempo = t1 - t0;
b) La funci ́on read(...). Tomando C como ejemplo:
t0 = ...
n = read(sockfd, buffer, 255);
t1 = ...
tiempo = t1 - t0;
c) Grafique y explique los resultados obtenidos. En particular, identifique
si las diferencias de tiempos son proporcionales a las cantidades de datos,
por ej: ¿write(...) con 1000 bytes toma 10 veces m ́as tiempo que con 100
bytes? Identifique si el tiempo de la funci ́on read() se mantiene constante,
dado que involucra siempre la misma cantidad de datos.
Proveer una descripci ́on m ́ınima de las computadoras usadas (CPU, RAM,
OS). Aclare si las computadoras est ́an en la misma red local o en dos redes
locales diferentes (en este  ́ultimo caso, es posible que deban modificar al
menos un router si ambas computadoras tienen acceso v ́ıa NAT). Pueden
utilizar las computadoras del aula o sus propias computadoras. Documentar
y entregar no solamente el c ́odigo de los procesos de comunicaciones sino
tambi ́en el propio experimento.
3) Desarrollar los mismos experimentos de comunicaciones que en el caso
anterior, con las mismas computadoras, pero ahora con la misma cantidad
de datos en un sentido y en el inverso (experimento “ping-pong”). El tiempo
total “de ida y vuelta” de los datos dividido por 2 es el que se utiliza para una
estimaci ́on del tiempo de mensajes en una direcci ́on. Comparar los tiempos
del ejercicio anterior con los que se obtienen en este experimento para cada
cantidad de bytes.
4) Teniendo en cuenta los experimentos de tiempos realizados, desarrollar
scripts para desplegar un ambiente de experimentaci ́on de comunicaciones
en una computadora con Vagrant para los siguientes escenarios:
a.- Dos m ́aquinas virtuales, cada una con un proceso de comunicaciones.
b.- Una m ́aquina virtual con uno de los procesos de comunicaciones y el
otro proceso de comunicaciones en el host.
En todos los casos deber ́ıan quedar los resultados disponibles para su pos-
terior an ́alisis. Se debe resolver el problema de “asincronismo”que genera
un error si el proceso “cliente” inicia la ejecuci ́on antes que el proceso “ser-
vidor”. Consultar con un asistente con IA cu ́al es ser ́ıa el mejor m ́etodo
experimental para estimar el tiempo de comuicaciones entre dos compu-
tadoras diferentes y comente si le parece correcto o puede identificar alg ́un
problema en la respuesta del asistente con IA.
5) Explique si los resultados de tiempo de comunicaciones del experimento
del ej. anterior se ven afectados por el orden de ejecuci ́on y las diferencias de
tiempo de ejecuci ́on iniciales de los programas utilizados ¿qu ́e sucede si por
alguna raz ́on hay una diferencia de 10 segundos en el inicio de la ejecuci ́on
entre un programa y otro? Sugerencia: incluya un “sleep” de 10 segundos
entre la ejecuci ́on de diferentes partes de los programas y comente.
Entrega de la pr ́actica (individual o en grupos de dos estudiantes como
m ́aximo):
- Se debe entregar un informe de lo realizado para cada ejercicio. Debe
tener un formato bien definido identificando materia, trabajo pr ́actico y
autor/es. Se debe entregar en formato electr ́onico con tipo de archivo .pdf,
en tama ̃no de hoja A4.
- Todos los resultados experimentales de tiempo deben presentarse en el
informe en formato de tabla y en formato gr ́afico. En el caso del formato
gr ́afico debe explicarse en el texto del informe qu ́e es lo m ́as importante que
se puede identificar en cada gr ́afico.
- Para cada programa modificado o generado para resolver los ejercicios,
debe explicarse el cambio o la implementaci ́on realizada en el informe para el
ejercicio que corresponde. Si bien el programa fuente puede estar comentado,
el cambio o la implementaci ́on realizada debe explicarse en el texto del
informe (no es aceptable “ver c ́odigo fuente” en el informe).
- Se debe entregar en formato electr ́onico tanto el informe como todo el
c ́odigo fuente usado/desarrollado