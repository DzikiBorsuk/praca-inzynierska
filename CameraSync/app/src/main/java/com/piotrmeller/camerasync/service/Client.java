package com.piotrmeller.camerasync.service;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;

public class Client {

    NetworkService service;
    DatagramSocket socket;
    InetSocketAddress serverAddress;
    boolean isConnected;
    //static final int clientPORT = 8080;
    private static final int MAX_UDP_DATAGRAM_LEN = 1500;
    //private String lastMessage = "";


    public Client(NetworkService service) {
        this.service = service;

    }

    public void connectToServer(final SocketAddress address) {


        try {
            socket = new DatagramSocket();
            socket.setSoTimeout(1000);
        } catch (SocketException e) {
            e.printStackTrace();
            disconnectFromServer();
            return;
        }


        final String connectMsg = "connect";
        final String ackMsg = "ack_connect";
        new Thread(new Runnable() {
            @Override
            public void run() {
                DatagramPacket packet = new DatagramPacket(connectMsg.getBytes(), connectMsg.length(), address);
                try {
                    socket.send(packet);
                } catch (Throwable e) {
                    e.printStackTrace();
                    disconnectFromServer();
                    return;
                }
                String message = null;
                byte[] buf = new byte[256];
                packet = new DatagramPacket(buf, buf.length, address);
                for (int i = 0; i < 5; i++) {
                    try {
                        socket.receive(packet);
                        message = new String(packet.getData(), 0,
                                packet.getLength());
                    } catch (SocketTimeoutException e) {
                        continue;
                    } catch (Exception e) {
                        e.printStackTrace();
                        disconnectFromServer();
                    }
                    if (message != null && message.equals(ackMsg)) {
                        serverAddress = (InetSocketAddress) address;
                        isConnected = true;
                        connected();
                        break;
                    }

                }
                if (!isConnected) {
                    disconnectFromServer();
                }
            }
        }).start();

    }

    private void connected() {
        new ClientReceiverThread().start();
    }

    public void disconnectFromServer() {
        if (socket != null) {
            socket.close();
            socket = null;
        }
        isConnected = false;
    }

    public class ClientReceiverThread extends Thread {
        @Override
        public void run() {
            String message;
            byte[] lmessage = new byte[MAX_UDP_DATAGRAM_LEN];
            DatagramPacket packet = new DatagramPacket(lmessage, lmessage.length);

            try {

                while (isConnected) {
                    try {
                        socket.receive(packet);
                        message = new String(lmessage, 0, packet.getLength());
                        if(message.equals("photo")){
                            service.callbacks.takePictureRequest();
                        }
                    } catch (SocketTimeoutException e) {
                        continue;
                    }
                }


            } catch (Throwable e) {
                e.printStackTrace();
            }


        }
    }

    public void send() {
        try {
            String a = "saas";
            // String addr = activity.editTextAddress.getText().toString();
            // InetAddress address = InetAddress.getByName(addr);
            //String add = address.getHostAddress();
            //DatagramPacket packet = new DatagramPacket(a.getBytes(), a.length(), address, Integer.parseInt(activity.editTextPort.getText().toString()));

            //socket.send(packet);

        } catch (Exception e) {
            e.printStackTrace();
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

}

