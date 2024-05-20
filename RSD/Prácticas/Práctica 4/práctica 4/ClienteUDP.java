import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.charset.StandardCharsets;
import java.io.IOException;
import java.net.DatagramPacket;
import java.util.Scanner;

public class ClienteUDP {

    public static void main(String[] args) throws IOException {
        DatagramSocket ds = new DatagramSocket();

        String texto;
        String direccionIP;
        String respuestaServidor;
        int puertoDestino;
        byte [] buffer;
        Scanner scanner;
        DatagramPacket dpMensaje;
        DatagramPacket dpRespuesta;

        while (true) {
            scanner = new Scanner(System.in);
            System.out.print("ClienteUDP: Introduzca la IP destino: ");
            direccionIP = scanner.nextLine();
            InetAddress ipDestino = InetAddress.getByName(direccionIP);
            System.out.print("ClienteUDP: Introduzca el puerto destino: ");
            puertoDestino = scanner.nextInt();
            scanner.nextLine(); //Consume \n
            System.out.println("ClienteUDP: IP --> " + direccionIP + " Port --> " + puertoDestino);
            
            System.out.print("ClienteUDP: Introduzca el mensaje a enviar: ");
            texto = scanner.nextLine();

            if (!(texto.charAt(0) >= '0' && texto.charAt(0) <= '9')) // No hay número en el texto
                break;
            
            buffer = texto.getBytes(StandardCharsets.UTF_8);
            

            dpMensaje = new DatagramPacket(buffer, buffer.length, ipDestino, puertoDestino);

            System.out.println("ClienteUDP: envío el paquete a " + dpMensaje.getAddress() + " al puerto " + dpMensaje.getPort());
            ds.send(dpMensaje);

            System.out.println("ClienteUDP: esparando paquete ACK . . .");
            buffer = new byte[1000];
            dpRespuesta = new DatagramPacket(buffer, buffer.length);
            ds.receive(dpRespuesta);
            System.out.println("ClienteUDP: datagrama de respuesta del servidor recibido.");

            respuestaServidor = new String(dpMensaje.getData(), dpMensaje.getOffset(), dpMensaje.getLength(), StandardCharsets.UTF_8);
            System.out.println("ClienteUDP: Mensaje: " + respuestaServidor+"\tIP:"+dpMensaje.getAddress()+"\tPort:"+dpMensaje.getPort());
        }

        ds.close();
        System.out.println("ClienteUDP: socket cerrado");
    }
}