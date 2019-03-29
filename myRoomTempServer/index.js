const express = require('express');
const fs = require('fs')
const connection = require('./tempDB.js');
const moment = require('moment-timezone')
const app = express()

connection.connect();

app.use('/js', express.static(__dirname + '/node_modules/bootstrap/dist/js')); // redirect bootstrap JS
app.use('/js', express.static(__dirname + '/node_modules/jquery/dist')); // redirect JS jQuery
app.use('/css', express.static(__dirname + '/node_modules/bootstrap/dist/css')); // redirect CSS bootstrap

app.get('/', function(req, res) {
    res.send('@20161596');
});

app.get('/dump', function(req, res) {
	var count = req.query.count || 120;
	count = Number(count);
	connection.query(`select * from sensors order by time DESC limit ${count}`, 
	 function(err, rows, fields){
		if(err)
			throw err;
		rows = rows.reverse()
		
		rows.forEach(function(row) {	
			var d = moment(new Date(row.time)).tz('Asia/Seoul').format('YYYYMMDD, HH:mm');
			var t = d + ', temperature = ' + String(row.value.toFixed(2)) + '\n';
			res.write(t);
		});
		res.end();
	});
});

app.get('/graph', function(req, res) {

	console.log('Connection from ' + req.connection.remoteAddress);
	var count = req.query.count || 120;
	count = Number(count);
	var html = fs.readFile('./front.html', function(err, html) {
		html = " " + html;	
		var qry = `select * from sensors order by time DESC limit ${count}`;
		connection.query(qry, function(err, rows, cols) {
			if (err) 
				throw err;
			rows = rows.reverse();
			var data = "";
			var comma = "";
			var dump = "";
			rows.forEach(function(row) {
				data += comma + `[new Date(${Date.parse(row.time)}),${row.value.toFixed(2)}]`;
				dump += moment(new Date(row.time)).tz('Asia/Seoul').format('YYYYMMDD, HH:mm')
					+ ', temperature = ' + String(row.value.toFixed(2)) + '\n';
				comma = ",";
			})
			var header = "data.addColumn('date', 'Date');"
			header += "data.addColumn('number', 'Temp');"

			var start = 0, end = 0;
			if (rows.length > 0) {
				start = moment(new Date(rows[0].time)).tz('Asia/Seoul');
				end = moment(new Date(rows[rows.length-1].time)).tz('Asia/Seoul');
			}
				
			html = html.replace("<%START%>", start);
			html = html.replace("<%END%>", end);
			html = html.replace("<%HEADER%>", header);
			html = html.replace("<%DATA%>", data);
			html = html.replace("<%DUMP%>", dump);
			res.writeHeader(200, {"Content-Type": "text/html"});
			res.write(html);
			res.end();
		});
	});
});

app.get('/data', function(req, res){
	var r = {};
	r.value = req.query.temp;
	connection.query('insert into sensors set ?', r, function(err, rows, cols) {
		if(err) 
			throw err;
		console.log(r);
	});
});

app.listen(3000, () => console.log('Start!'));
