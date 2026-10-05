/*
 * EchoServer.java
 * Just receives some data and sends back a "message" to a client
 *
 * Usage:
 * java Server port
 */

import java.io.*;
import java.net.*;

public class Server
{
  public static void main(String[] args) throws IOException
  {


    /* 1. Validar argumentos (programa + puerto) */
    if ((args.length != 1) || (Integer.valueOf(args[0]) <= 0) )
    {
      System.out.println("1 arguments needed: port");
      System.exit(1);
    }


    /* 2. Crear el ServerSocket (socket + bind + listen) */
    ServerSocket serverSocket = null;    
    try
    {
      serverSocket = new ServerSocket(Integer.valueOf(args[0]));
    } 
    catch (Exception e)
    {
      System.out.println("Error on server socket");
      System.exit(1);
    }


    /* 3. Bloquearse a la espera del cliente con accept() */
    Socket connected_socket = null;
    try
    {
      connected_socket = serverSocket.accept();
    }
    catch (IOException e)
    {
      System.err.println("Error on Accept");
      System.exit(1);
    }


    /* 4. Obtener los streams de entrada/salida */
    DataInputStream fromclient;
    DataOutputStream toclient;
    fromclient = new DataInputStream(connected_socket.getInputStream());
    toclient   = new DataOutputStream(connected_socket.getOutputStream());


    /* 5. Recibir datos del cliente con read() */
    byte[] buffer;
    buffer = new byte[256];
    fromclient.read(buffer);
    String str = new String(buffer);
    System.out.println("Here is the message: " +  str);


    /* 6. Enviar una respuesta al cliente con write() */
    String strresp = "I got your message";
    System.out.println("strrsp " + strresp);
    buffer = strresp.getBytes();
    toclient.write(buffer, 0, buffer.length);


    /* 7. Enviar una respuesta al cliente con write() */
    fromclient.close();
    toclient.close();
    connected_socket.close();
    serverSocket.close();


  }
}
