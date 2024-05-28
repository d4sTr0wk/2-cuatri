import java.io.IOException;
import java.net.*;
import java.util.Scanner;

public class ServidorUDP_esqueleto {
    public static void main(String[] args) throws IOException {
        // DATOS DEL SERVIDOR
        int port = 12345;
        // int port = Integer.parseInt(args[0]);
        //Scanner scanner = new Scanner(System.in);
        // int port = scanner.nextInt();

        //SOCKET
        DatagramSocket server = null;

        server = new DatagramSocket(port);

        // FUNCIÓN
        while (true)
        {
            // Crear e inicializar datagrama vacío
            DatagramPacket data = new DatagramPacket(new byte[500], 500);

            // Recibir datagrama
            server.receive(data);

            // Obtener texto recibido
            String line = new String(data.getData());

            // Mostrar por pantalla metadatos datagrama
            System.out.println("IP: " + data.getAddress().getHostAddress() + " Puerto: " + data.getPort() + " Texto: " + line);

            line = "Datos recibidos";

            // Crear datagrama de respuesta
            byte[] array = line.getBytes();
            DatagramPacket respuesta = new DatagramPacket(array, array.length, data.getAddress(), data.getPort());

            // Enviar datagrama de respuesta
            server.send(respuesta);
        }
    }
}
