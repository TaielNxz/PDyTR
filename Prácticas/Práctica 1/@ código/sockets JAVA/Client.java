/*
 * Client.java
 * Just sends stdin read data to and receives back some data from the server
 *
 * usage:
 * java Client serverhostname port
 */

import java.io.*;
import java.net.*;

public class Client
{
  public static void main(String[] args) throws IOException
  {


    /* 1. Validar argumentos (host + puerto) */
    if ((args.length != 2) || (Integer.valueOf(args[1]) <= 0) )
    {
      System.out.println("2 arguments needed: serverhostname port");
      System.exit(1);
    }

    
    /* 2. Crear el Socket y conectarse al servidor */
    Socket socketwithserver = null;
    try
    { 
      socketwithserver = new Socket(args[0], Integer.valueOf(args[1]));
    }
    catch (Exception e)
    {
      System.out.println("ERROR connecting");
      System.exit(1);
    } 


    /* 3. Obtener los streams de entrada/salida */
    DataInputStream  fromserver;
    DataOutputStream toserver;
    fromserver = new DataInputStream(socketwithserver.getInputStream());
    toserver   = new DataOutputStream(socketwithserver.getOutputStream());


    /* 4. Leer mensaje del usuario por consola */
    Console console  = System.console();
    String inputline = console.readLine("Please enter the message: ");


    /* 5. Enviar el mensaje al servidor */
    byte[] buffer;
    buffer = inputline.getBytes();
    toserver.write(buffer, 0, buffer.length);
    

    /* 6. Enviar el mensaje al servidor */
    buffer = new byte[256];
    fromserver.read(buffer);
    String resp = new String(buffer);
    System.out.println(resp);
    

    /* 7. Cerrar streams y socket */
    fromserver.close();
    toserver.close();
    socketwithserver.close();


  }
}
