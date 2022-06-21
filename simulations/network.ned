simple Server {
    gates:
        input  in[];
        output out[];
}


network Network {
    parameters:
    	int serversCount @prompt("Number of server") = default(5);

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