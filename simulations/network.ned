simple Server {
    gates:
        input  in[];
        output out[];
}


network Network {
    parameters:
        int serversCount @prompt("Number of servers") = default(5);

        @signal[messageReceived];
        @signal[messageSent];
        @statistic[receivedMessages](source=messageReceived; record=count);
        @statistic[sentMessages](source=messageSent; record=count);

    types:
        channel serverChannel extends ned.DelayChannel { delay = 10ms; }

    submodules:
        servers[serversCount]: Server {}

    connections:       
        for i=0..serversCount-1, for j=0..serversCount-1 {
            servers[i].out++ --> serverChannel --> servers[j].in++;
            servers[i].in++  <-- serverChannel <-- servers[j].out++;
        }
}
