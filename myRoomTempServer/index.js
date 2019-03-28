const express = require('express');
const fs = require('fs')
const mysql = require('mysql')
const moment = require('moment-timezone')
const app = express()

var connection = mysql.createConnection({
	host: 'localhost',
	user: 'me',
	password: 'mypassword',
	database: 'tempDB',
})

connection.connect();

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
