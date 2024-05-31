import java.io.*;
import java.net.*;
import java.util.Scanner;

public class ClientTCP {
    public static void main(String[] args) throws IOException {
        //DATOS DEL SERVIDOR
        String serverName = "127.0.0.1";
        int serverPort = 12345;

        // String serverName = args[0];
        //int serverPort = Integer.parseInt(args[1]);

        // SOCKET
        Socket serviceSocket = null;

        PrintWriter out = null;
        BufferedReader in = null;

        serviceSocket = new Socket(serverName, serverPort);

        InputStream is = serviceSocket.getInputStream();
        InputStreamReader isr = new InputStreamReader(is);
        in = new BufferedReader(isr);
        OutputStream os = serviceSocket.getOutputStream();
        OutputStreamWriter osr = new OutputStreamWriter(os);
        out = new PrintWriter(osr, true);

        // Obtener texto por teclado
        BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in));
        String userInput;

        System.out.println("Introduzca el texto: ");
        userInput = stdIn.readLine();

        while (Character.isDigit(userInput.charAt(0))) {
            // Enviar texto al servidor a través del socket
            out.println(userInput);

            // Recibir texto enviado por el servidor
            String line = null;
            line = in.readLine();

            System.out.println("respuesta recibida del servidor: " + line);

            System.out.println("Introduzca el texto: ");
            userInput = stdIn.readLine();
        }
        // Enviar FINISH al servidor
        out.println("FINISH");

        // Recibir el OK del servidor
        System.out.println(in.readLine());

        in.close();
        out.close();
        serviceSocket.close();
    }
    
}
