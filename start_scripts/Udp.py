import socket
import threading
import Queue

class Udpjieshou(threading.Thread):
    def __init__(self, ip, port, que, running=True):
        super(Udpjieshou, self).__init__()
        self.ip = ip
        self.port = int(port)
        self.que = que
        self.running = running
        self.BUFFZIE = 1024*1024
        self.ip_port = (self.ip, self.port)
        print("ip_port:",self.ip_port)
        self.srv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.srv.bind(self.ip_port)
        self.srv.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, self.BUFFZIE)
        bsize = self.srv.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
        print("bsize:", bsize)

    def run(self)->None:
        str = "Udpjieshou run."
        print(str)
        while self.running:
            data, client_addr = self.srv.recvfrom(self.BUFFZIE)
            self.que.put(data.decode('utf-8'))
            print('udp:',data)
        self.srv.close()


class Udpfasong:
    def __init__(self, ip, port) -> None:
        self.ip = ip
        self.port = port
        self.ip_port = (ip, int(port))
        self.srv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        bsize = self.srv.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
        self.srv.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024)
        bsize = self.srv.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
        self.BUFFZIE = 1024 * 1024

    def Send(self, msg):
        ret = self.srv.sendto(msg.encode('utf-8'), self.ip_port)
        return ret

    def Recv(self):
        data, client_addr = self.srv.recvfrom(self.BUFFZIE)
        print("recv:", client_addr, data.decode('utf-8'))
        return data.decode('utf-8')

class UdpRecv:
    def __init__(self, ip, port) -> None:
        self.ip = ip
        self.port = port
        self.ip_port = (ip, int(port))
        self.srv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.srv.bind(self.ip_port)
        bsize = self.srv.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
        self.srv.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024)
        bsize = self.srv.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
        self.BUFFZIE = 1024 * 1024

    def Send(self, msg, port):
        ret = self.srv.sendto(msg.encode('utf-8'), (self.client_addr[0], port))
        return ret

    def Recv(self):
        data, self.client_addr = self.srv.recvfrom(self.BUFFZIE)
        print("recv:", self.client_addr, data.decode('utf-8'))
        return data.decode('utf-8')
        
if __name__ == '__main__':
    udp = Udpfasong("192.168.1.50", 6000)
    ret = udp.Send("hello")
    print(ret)
    udp.Recv()