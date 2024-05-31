import java.io.*;
import java.net.*;
import java.util.Scanner;

public class ServerTCP {
    public static void main(String[] args) throws IOException {
        int port = 12345;
        int tamanoCola = 1;

        ServerSocket server = null;
        Socket client = null;

        BufferedReader in = null;
        PrintWriter out = null;

        server = new ServerSocket(port, tamanoCola);

        while (true)
        {
            System.out.println("ServidorTCP: esperando clientes");
            client = server.accept();

            InputStream is = client.getInputStream();
            InputStreamReader isr = new InputStreamReader(is);
            in = new BufferedReader(isr);
            OutputStream os = client.getOutputStream();
            OutputStreamWriter osr = new OutputStreamWriter(os);
            out = new PrintWriter(osr, true);

            boolean salir = false;
            while(!salir)
            {
                System.out.println("ServidorTCP: cliente conectado /" + client.getLocalSocketAddress());
                String line = null;
                line = in.readLine();
                System.out.println("ServidorTCP: recibido por el cliente: " + line);

                if (line.compareTo("FINISH") != 0)
                {
                    //procesamiento del string
                    out.println(line);
                    out.flush();
                } else {
                    salir = true;
                    out.println("OK");
                }
            }
            in.close();
            out.close();
            client.close();
        }

    }
}
