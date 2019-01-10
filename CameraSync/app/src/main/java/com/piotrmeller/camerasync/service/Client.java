package com.piotrmeller.camerasync.service;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Deque;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

public class Client {

    NetworkService service;
    DatagramSocket socket;
    DatagramSocket syncSocket;
    InetSocketAddress serverAddress;
    InetSocketAddress serverSyncAddress;
    long serverTimestamp = System.currentTimeMillis();
    long avgDeltaTimestamp = 0;
    boolean isConnected;
    int syncPort;
    Deque<Long> deltaTimestamps = new LinkedList<>();
    //static final int clientPORT = 8080;
    private static final int MAX_UDP_DATAGRAM_LEN = 256;
    //private String lastMessage = "";


    public Client(NetworkService service) {
        this.service = service;

    }

    public void connectToServer(final SocketAddress address) {

        disconnectFromServer();
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

                String message = null;
                byte[] buf = new byte[MAX_UDP_DATAGRAM_LEN];
                DatagramPacket receivePacket = new DatagramPacket(buf, buf.length, address);
                for (int i = 0; i < 5; i++) {
                    try {
                        socket.send(packet);
                        socket.receive(receivePacket);
                        message = new String(receivePacket.getData(), 0,
                                receivePacket.getLength());
                    } catch (SocketTimeoutException e) {
                        continue;
                    } catch (Exception e) {
                        e.printStackTrace();
                        //disconnectFromServer();
                    }


                    if (message != null) {
                        String[] message_parts = message.split("%");

                        if (message_parts.length > 1) {

                            if (message_parts[0].equals(ackMsg)) {
                                serverAddress = (InetSocketAddress) address;
                                isConnected = true;
                                syncPort = Integer.parseInt(message_parts[1]);
                                serverSyncAddress = new InetSocketAddress(serverAddress.getAddress(), syncPort);
                                connected();
                                break;
                            }
                        }
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
        new ClientSyncThread().start();
    }

    public void disconnectFromServer() {
        if (socket != null) {
            socket.close();
            socket = null;
        }
        if (syncSocket != null) {
            syncSocket.close();
            socket = null;
        }
        isConnected = false;
        deltaTimestamps.clear();
    }

    public class ClientReceiverThread extends Thread {
        @Override
        public void run() {
            String message;
            byte[] lmessage = new byte[MAX_UDP_DATAGRAM_LEN];
            DatagramPacket packet = new DatagramPacket(lmessage, lmessage.length);
            //packet = new DatagramPacket()

            try {

                while (isConnected) {
                    try {
                        socket.receive(packet);
                        message = new String(lmessage, 0, packet.getLength());
                        String[] message_parts = message.split("%");
                        if (message_parts.length > 1) {
                            if (message_parts[0].equals("photo")) {
                                long targetTimestamp = Long.parseLong(message_parts[1]) + avgDeltaTimestamp;
                                service.callbacks.takePictureRequest(targetTimestamp - System.currentTimeMillis());
                            }
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

    public class ClientSyncThread extends Thread {
        @Override
        public void run() {
            try {
                syncSocket = new DatagramSocket(syncPort);
            } catch (SocketException e) {
                e.printStackTrace();
                disconnectFromServer();
                return;
            }

            String message;
            //byte[] lmessage = new byte[MAX_UDP_DATAGRAM_LEN];
            ByteBuffer buffer = ByteBuffer.allocate(8);
            buffer.putLong(0);
            DatagramPacket packet = new DatagramPacket(buffer.array(), 0, buffer.limit(), serverSyncAddress);
            try {
                syncSocket.setSoTimeout(1000);
            } catch (SocketException e) {
                e.printStackTrace();
            }
            long start, stop;
            while (isConnected) {
                try {
                    //start = System.currentTimeMillis();
                    start = System.nanoTime();
                    syncSocket.send(packet);
                    syncSocket.receive(packet);
                    long stop2 = System.currentTimeMillis();
                    stop = System.nanoTime();
                    long delay = ((stop - start) / 2) / 1000000;
                    buffer = ByteBuffer.wrap(packet.getData(), 0, packet.getLength());
                    serverTimestamp = buffer.getLong();
                    //avgDeltaTimestamp = stop2 - (serverTimestamp + delay);

                    deltaTimestamps.addLast(stop2 - (serverTimestamp + delay));
                    if (deltaTimestamps.size() > 25) {
                        deltaTimestamps.pollFirst();
                    }

                    Iterator<Long> itr = deltaTimestamps.iterator();
                    Long value = 0L;
                    while (itr.hasNext()) {
                        // next() returns the next element in the iteration
                        //System.out.println(itr.next());
                        value = value + itr.next();

                    }

                    avgDeltaTimestamp = value / deltaTimestamps.size();

                    buffer.clear();

                    Thread.sleep(100);

                } catch (SocketTimeoutException e) {
                    continue;
                } catch (IOException e) {
                    e.printStackTrace();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

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

