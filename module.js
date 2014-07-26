/* wiringServer NodeJs module */
function wiringServer() {
	this.onConnect = [];
	this.callbackId = 1;
	this.callbacks = {};
};

wiringServer.prototype.functions = {
	DHT11: 0
};

wiringServer.prototype.connect = function() {
	var self = this;
	this.connecting = true;
	this.socket = require('net').connect({path: require('path').resolve(__dirname, 'server.sock')}, function() {
		delete self.connecting;
		self.onConnect.forEach(function(cb) {
			cb();
		});
		self.onConnect = [];
	});
	this.socket.on('data', function(data) {
		try {
			data = JSON.parse(data);
		} catch(e) {
			return;
		}
		if(data.callback && self.callbacks[data.callback]) {
			var callbackId = data.callback;
			delete data.callback;
			self.callbacks[callbackId](data);
			delete self.callbacks[callbackId];
		}
	});
	this.socket.on('close', function() {
		delete self.connecting;
		delete self.socket;
	});
	this.socket.on('error', function() {

	});
};

wiringServer.prototype.request = function(func, callback) {
	if(!this.socket) {
		var self = this;
		this.onConnect.push(function() {
			self.request(func, callback);
		});
		return !this.connecting && this.connect();
	}
	var callbackId = this.callbackId++;
	this.callbacks[callbackId] = callback;
	this.socket.write(String.fromCharCode(func) + callbackId + '\n');
};

module.exports = wiringServer;
