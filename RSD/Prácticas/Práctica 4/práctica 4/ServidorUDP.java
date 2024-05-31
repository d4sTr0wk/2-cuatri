import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.nio.charset.StandardCharsets;
import java.util.Scanner;

public class ServidorUDP {

    public static void main(String[] args) throws IOException {

        Scanner scanner = new Scanner(System.in);
        System.out.print("ServidorUDP: Introduzca el puerto destino: ");
        int port = scanner.nextInt();

        DatagramSocket ds = new DatagramSocket(port);
        DatagramPacket dpMensaje;
        DatagramPacket dpRespuesta;
        String texto;
        String respuesta;
        String [] palabras;
        byte[] buffer;
        int indexNumber;
        int length;

        while (true){
            respuesta = "";
            System.out.println("ServidorUDP: Esperando peticiones o mensajes del cliente . . .");
            buffer = new byte[1000];
            dpMensaje = new DatagramPacket(buffer, buffer.length);
            ds.receive(dpMensaje);

            texto = new String(dpMensaje.getData(), dpMensaje.getOffset(), dpMensaje.getLength(), StandardCharsets.UTF_8);
            System.out.println("ServidorUDP: Mensaje: " + texto+"\tIP:"+dpMensaje.getAddress()+"\tPort:"+dpMensaje.getPort());

            /*TRATAMIENTO DEL MENSAJE RECIBIDO */
            length = 0;
            for(indexNumber = 0; texto.charAt(indexNumber) >= '0' && texto.charAt(indexNumber) <= '9'; indexNumber++ )
            {
                length = length*10 + (int)texto.charAt(indexNumber) - '0';
            }
            texto = texto.replaceFirst("^[0-9]+", "");
            palabras = texto.split(" ");
            for (String palabra : palabras) 
            {
                if (palabra.length() > length)
                {
                    respuesta += palabra + " ";
                }
            }

            buffer = respuesta.getBytes(StandardCharsets.UTF_8);

            System.out.println("ServidorUDP: Envío respuesta: " + respuesta);
            dpRespuesta = new DatagramPacket(buffer, buffer.length, dpMensaje.getAddress(), dpMensaje.getPort());
            ds.send(dpRespuesta);
            System.out.println("ServidorUDP: Respuesta enviada.");
        }
        

    }
}