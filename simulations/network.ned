simple Server {
    gates:
        input  in[];
        output out[];
}


network Network {
    parameters:
        int serversCount @prompt("Number of servers") = default(5);

        @signal[messageReceivedBeforeElection];
        @signal[messageSentBeforeElection];
        @statistic[receivedMessagesUntilElection](source=messageReceivedBeforeElection; record=count);
        @statistic[sentMessagesUntilElection](source=messageSentBeforeElection; record=count);

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
