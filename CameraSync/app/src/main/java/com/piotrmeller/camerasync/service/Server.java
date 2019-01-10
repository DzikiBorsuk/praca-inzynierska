package com.piotrmeller.camerasync.service;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Enumeration;

public class Server {
    private NetworkService service;
    private DatagramSocket socket;
    private DatagramSocket syncSocket;
    private boolean isServerOpen = false;
    //static final int serverPORT = 8080;
    private static final int MAX_UDP_DATAGRAM_LEN = 256;
    private ArrayList<InetSocketAddress> clientsList = new ArrayList<>();
    //private String lastMessage = "";

    public Server(NetworkService service) {
        this.service = service;
    }

    public void startServer() {
        closeServer();
        try {
            socket = new DatagramSocket();
            syncSocket = new DatagramSocket();
            String address = socket.getLocalAddress().getHostAddress();
            int port = socket.getLocalPort();

            isServerOpen = true;

            Thread thread = new ServerReceiverThread();
            thread.start();
            new ServerSyncThread().start();
            //activity.infoip.setText(address + " : " + Integer.toString(port));
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    public void closeServer() {
        if (socket != null) {
            socket.close();
            socket = null;
        }
        if(syncSocket!=null)
        {
            syncSocket.close();
            socket=null;
        }
        isServerOpen = false;
        clientsList.clear();

    }

    public ArrayList<InetSocketAddress> getClientsList() {
        return clientsList;
    }

    public boolean isServerOpen() {
        return isServerOpen;
    }

    public class ServerReceiverThread extends Thread {
        @Override
        public void run() {
            String message;
            byte[] lmessage = new byte[MAX_UDP_DATAGRAM_LEN];
            DatagramPacket packet = new DatagramPacket(lmessage, lmessage.length);
            while (isServerOpen) {
                try {
                    socket.receive(packet);

                    InetSocketAddress address = (InetSocketAddress) packet.getSocketAddress();

                    message = new String(packet.getData(), 0,
                            packet.getLength());

                    if (message.equals("connect")) {
                        if (!clientsList.contains(address)) {
                            clientsList.add(address);

                            new Thread(() -> {

                                String port = Integer.toString(syncSocket.getLocalPort());
                                final String response = "ack_connect%"+port;
                                DatagramPacket packet1 = new DatagramPacket(response.getBytes(), response.length(), address);
                                try {
                                    socket.send(packet1);
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            }).start();
                        }
                    }

                } catch (SocketTimeoutException e) {
                    // timeout exception.

                } catch (Throwable e) {
                    e.printStackTrace();
                }
            }

        }
    }

    public class ServerSyncThread extends Thread {

        @Override
        public void run() {

            ByteBuffer buffer = ByteBuffer.allocate(8);
            DatagramPacket packet = new DatagramPacket(buffer.array(),0,buffer.limit());
            while(isServerOpen)
            {
                try {
                    syncSocket.receive(packet);
                    SocketAddress address = packet.getSocketAddress();
                    buffer.clear();
                    buffer.putLong(System.currentTimeMillis());
                    packet = new DatagramPacket(buffer.array(),0,buffer.limit(),address);
                    syncSocket.send(packet);
                    buffer.clear();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }


    public void broadcast() {
        long targetTimestamp = System.currentTimeMillis()+500;
        for (InetSocketAddress address : clientsList) {
            new Thread(() -> {

                String message = "photo%"+Long.toString(targetTimestamp);


                DatagramPacket packet = new DatagramPacket(message.getBytes(), message.length(), address);
                try {
                    socket.send(packet);
                } catch (Exception e) {
                    e.printStackTrace();
                }


            }).start();
        }
    }

    public void send() {
        try {
            Thread t = new Thread(new Runnable() {
                @Override
                public void run() {
                    //TODO implementation
                    // Insert some method call here.
                }
            });
            t.start();

        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    public InetSocketAddress getServerAddress() {
        //String ip = "";
        InetSocketAddress address = null;
        if (socket != null) {
            try {
                Enumeration<NetworkInterface> enumNetworkInterfaces = NetworkInterface
                        .getNetworkInterfaces();
                while (enumNetworkInterfaces.hasMoreElements()) {
                    NetworkInterface networkInterface = enumNetworkInterfaces
                            .nextElement();
                    Enumeration<InetAddress> enumInetAddress = networkInterface
                            .getInetAddresses();
                    while (enumInetAddress.hasMoreElements()) {
                        InetAddress inetAddress = enumInetAddress
                                .nextElement();

                        if (inetAddress.isSiteLocalAddress()) {
                            address = new InetSocketAddress(inetAddress, socket.getLocalPort());
                        }
                    }
                }

            } catch (SocketException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                //ip += "Something Wrong! " + e.toString() + "\n";
                return null;
            }
        }
        return address;
    }
}

