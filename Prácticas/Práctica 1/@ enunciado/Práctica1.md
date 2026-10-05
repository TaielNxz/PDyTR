## Programaci´on Distribuida y Tiempo Real

## Pr´actica 1

- 1) Teniendo en cuenta la comunicaci´on con sockets (puede usar tanto los ejemplos provistos como tambi´en otras fuentes de informaci´on, que se sugiere referenciar de manera expl´ıcita):

- a.- Identifique similitudes y diferencias entre los sockets en C y en Java.

- modelo c/s? Nota: corroborar con la clase donde se explica el modelo C/S. b.- ¿Por qu´e puede decirse que los ejemplos no son representativos del

- c.- ¿Qu´e cambio/s deber´ıan hacerse para que “cliente” provisto funcione como “servidor” y el “servidor” provisto funcione como “cliente”? Nota: corroborar con la clase donde se explica el modelo C/S.

- 2) Desarrolle experimentos para quede claro que:

- a.- Aunque un proceso “servidor” programado en C haya obtenido un socket y hecho un bind() no va a haber ning´un otro proceso que pueda hacer una conexi´on con ´el a menos que se haya hecho el listen() ¿Por qu´e este mismo experimento no podr´ıa hacerse programando en Java?

- b.- Un proceso “cliente” puede tener una conexi´on con el proceso “ser- vidor” aunque el “servidor” no haya ejecutado la operaci´on accept().

- 3) Modifique el c´ odigo (programa C o Java o ambos) para que la cantidad de datos que se comunican sea de 10^1, 10^2, 10^3, 10^4, 10^5 y 10^6 y contengan bytes asignados directamente en el programa (sin leer de teclado ni mostrar en pantalla los datos del buffer). Tenga en cuenta que el valor de retorno de la llamada a read(...) tanto en C como en Java retornan la cantidad de bytes efectivamente le´ıda y puede ser menor a la que se le indica/pedida como par´ametro. Explique c´ omo verifica el correcto funcionamiento de lo desarrollado. El env´ıo debe realizarse en una ´unica llamada a la funci´on correspondiente a menos que el valor de retorno indique que hay datos pen- dientes de env´ıo.

- 4) Con el programa que desarroll´o en el ejercicio anterior para comunicar 101, 102, 103, 104, 105 y 106 bytes y utilizando el vagrantfile entregado por la c´ atedra, desarrolle y muestre un experimento que registre el tiempo que toma en el cliente (cada proceso en una vm diferente o un proceso en una vm y el otro en el host de las vm):

- ejemplo: a) La funci´on write(...) para cada cantidad de datos. Tomando C como

```
t0 = ...
write(...)
t1 = ...
tiempo = t1 - t0;
```


- b) La funci´on read(...). Tomando C como ejemplo:

```
t0 = ...
n = read(sockfd, buffer, 255);
t1 = ...
tiempo = t1 - t0;
```

Nota: dependiendo del lenguaje utilizado es posible que deba incluir en el vagrantfile lo necesario para compilar y ejecutar los procesos en las vm y eventualmente en el host si uno de los procesos va a estar en ejecuci´on en la computadora host.

- c) Grafique y explique los resultados obtenidos. En particular, identifique si las diferencias de tiempos son proporcionales a las cantidades de datos, por ej: ¿write(...) con 1000 bytes toma 10 veces m´as tiempo que con 100 bytes? Identifique si el tiempo de la funci´on read() se mantiene constante, dado que involucra siempre la misma cantidad de datos.

- 5) ¿Podr´ıa implementar un servidor de archivos remotos utilizando sockets? Describa brevemente la interfaz y los detalles que considere m´as importantes del dise˜no. No es necesario implementar.

- 6) Explique y justifique brevemente ventajas y desventajas de un servidor con estado respecto de un servidor sin estados. Considere las respuestas de diferentes asistentes de IA y analice

- tes de IA. a) Similitudes y diferencias entre las respuestas de los diferentes asisten-

- b) Elabore su propio an´alisis.
