<?php
include 'conn.php';
header("Content-Type:text/html;charset=utf-8");
session_start();
//获取name和password
 $name = $_POST["username"];
 $password = $_POST["password"];
 
 $sql = "SELECT * FROM amdin WHERE Admin_ID='{$name}'";
 //执行查询操作、处理结果集
 $result = mysqli_query($link, $sql);
 if (!$result) {
     exit('查询sql语句执行失败。错误信息：'.mysqli_error($link));  // 获取错误信息
 }
 $data = mysqli_fetch_all($result, MYSQLI_ASSOC);
 
 //将二维数数组转化为一维数组
 foreach ($data as $key => $value) {
   foreach ($value as $k => $v) {
     $arr[$k]=$v;
   }
 }
 
 

//判断是否为空
if($name != null && $password != null){
//比对与数据库中的值
if($name==$arr['Admin_ID']&& $password==$arr['Admin_PSW']){
	
	
	 
 
	  header("Location:index.php");//跳转成功
}else{
	 header("Location:error.php");//跳转失败
	
}
 
}
else{
	 header("Location:error0.php");//跳转失败
	
}
mysqli_close($link);
?>
