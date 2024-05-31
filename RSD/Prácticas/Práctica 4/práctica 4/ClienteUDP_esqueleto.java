import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.*;

public class ClienteUDP_esqueleto {
    public static void main(String[] args) throws IOException{
        // DATOS DEL SERVIDOR
        String serverName = "127.0.0.1";
        int serverPort = 12345;

        //String serverName = args[0];
        // int serverPort = Integer.parseInt(args[1]);

        DatagramSocket serviceSocket = null;

        serviceSocket = new DatagramSocket();

        // INICIALIZA ENTRADA POR TECLADO
        BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in));
        String userInput;
        System.out.println("Introduce el texto a enviar (END para terminar): ");
        userInput = stdIn.readLine();

        while(userInput.compareTo("END") != 0)
        {
            byte array[] = userInput.getBytes();
            DatagramPacket data = new DatagramPacket(array, array.length, InetAddress.getByName(serverName), serverPort);

            serviceSocket.send(data);
            System.out.println("ClienteUDP: esperando respuesta del servidor");

            DatagramPacket answer = new DatagramPacket(new byte[500], 500);

            serviceSocket.receive(answer);
            String line = new String(answer.getData());

            System.out.println("ClienteUDP: respuesta recibida: " + line);
            System.out.println("Introduzca el texto a enviar (END para terminar): ");

            userInput = stdIn.readLine();
        }
        System.out.println("ClienteUDP: cerrando cliente");

        serviceSocket.close();

        System.out.println("ClienteUDP: cerrado");
    }
}
