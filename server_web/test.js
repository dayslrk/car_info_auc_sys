


function login(){
    var username=document.getElementById("username");
    var pass=document.getElementById("password");
    var mysql=require('mysql');
    var connection=mysql.createConnection({
    host :'localhost',
    user :'root',
   password :'3306',
    database:'test',
})
connection.connect();
var sql='select *
    if(username.value==""){
        alert("请输入用户名");

    }
    else if(pass.value==""){
        alert("请输入密码");

    }
    else if(username.value=="root"&&pass.value=="123456"){
        window.location.href="index.php";
    }
    else{
        alert("请输入正确的用户名和密码");
    }
}
