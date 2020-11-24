<?php
//连接数据库
include 'conn.php';

// 获取增加的学生信息
$id=$_POST['id'];
$name = $_POST['name'];
$key = $_POST['key'];


	//编写预处理sql语句
	$sql = "INSERT INTO amdin(Admin_ID,Admin_PSW,Admin_Name) VALUES(?, ?, ?)";
	//预处理SQL模板
	$stmt = mysqli_prepare($link, $sql);
	// 参数绑定，并为已经绑定的变量赋值
	mysqli_stmt_bind_param($stmt, 'sss', $id,$name, $key);
	

	if ($name) {
		// 执行预处理（第1次执行）
		$result = mysqli_stmt_execute($stmt);
		//关闭连接
		mysqli_close($link);

		if ($result) {
    		
			header("Location:index.php");
		}else{
			exit('添加配件sql语句执行失败。错误信息：' . mysqli_error($link));
		}
	}else{
	
		echo "添加用户失败！<br><br>";
		header('Refresh: 3; url=index.php');  //3s后跳转

    	
	}

  

