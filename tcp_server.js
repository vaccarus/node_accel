
//In the node.js intro tutorial (http://nodejs.org/), they show a basic tcp 
//server, but for some reason omit a client connecting to it.  I added an 
//example at the bottom.
//Save the following server in example.js:


var net = require('net');

console.log('Listening on TCP server at 10.20.0.184:1337');

var server = net.createServer(function(sock) {
	// We have a connection - a socket object is assigned to the connection automatically
    console.log('CONNECTED: ' + sock.remoteAddress +':'+ sock.remotePort);
	
    // Add a 'data' event handler to this instance of socket
	// .toString('hex'))
    sock.on('data', function(data) {
        
		var serialData = JSON.parse(data);
		
        console.log('DATA ' + sock.remoteAddress + ': ' + serialData.x);
        
		// Write the data back to the socket, the client will receive it as data from the server
        sock.write('You said "' + data + '"');
        
    });
	
	sock.on('error', (err) => {
		// handle errors here
		throw err;
	});
	
	// Add a 'close' event handler to this instance of socket
    sock.on('close', function(data) {
        console.log('CLOSED: ' + sock.remoteAddress +' '+ sock.remotePort);
    });
	
	
}).listen(1337, '10.20.0.184');



//And connect with a tcp client from the command line using netcat, the *nix 
//utility for reading and writing across tcp/udp network connections.  I've only 
//used it for debugging myself.
//$ netcat 127.0.0.1 1337
//You should see:
//> Echo server