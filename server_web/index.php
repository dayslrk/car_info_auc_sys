<?php
//连接数据库
include 'conn.php';
$page=1;//设置没有PAGE参数，PAGE自动为1
$pageno=5;// 设置每一页的显示条数
$pagestart=($page-1)*$pageno;
//编写查询sql语句
$sql = "SELECT * FROM car";
//执行查询操作、处理结果集
$result = mysqli_query($link, $sql);
if (!$result) {
    exit('查询sql语句执行失败。错误信息：'.mysqli_error($link));  // 获取错误信息
}
$data = mysqli_fetch_all($result, MYSQLI_ASSOC);

//编写查询数量sql语句
$sql = "SELECT COUNT(*) FROM car";
//执行查询操作、处理结果集
$n = mysqli_query($link, $sql);
if (!$n) {
    exit('查询数量sql语句执行失败。错误信息：'.mysqli_error($link));  // 获取错误信息
}
$num = mysqli_fetch_assoc($n);
//将一维数组的值转换为一个字符串
$num = implode($num);



?>

<html>
	<head>
		<meta charset="UTF-8">
		<title>车辆信息管理系统</title>
	</head>
	<style type="text/css">
		body {
			background-image: url(student.jpg);
			background-size: 100%;
		}

		.wrapper {
			width: 1000px;
			margin: 20px auto;
		}

		h1 {
			text-align: center;
		}

		.add {
			margin-bottom:20px;	
			-moz-box-sizing:border-box;
			-webkit-box-sizing:border-box;
			-o-box-sizing:border-box;
			-moz-transition:all 0.3s linear;
			-webkit-transition:all 0.3s linear;
			
		}
		.add:hover{
		
			border:solid 1px rgba(102,146,191,0.68);
			background-color:#5CACEE;
			padding:6px;
			border-radius:5px;
			box-shadow:7px 15px 30px #285a63;
		}
		.add a {
			text-decoration: none;
			color: #fff;
			background-color: #00CCFF;
			padding: 6px;
			border-radius: 5px;
		}

		td {
			text-align: center;
		}
	</style>
	<body>
		<div class="wrapper">
			<h1>车辆信息管理系统</h1>
			<div class="add"><br></br>
				<a href="addStudent.html">添加车辆信息</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="searchStudent.html">查找车辆</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="authenticate.html">认证</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="addtype.html">更新车辆类型</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="adddev.html">添加配件</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="up_carinfo.html">导入车辆表</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="up_typeinfo.html">导入车型表</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="up_devinfo.html">导入配件表</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<br></br>
				<a href="add_user.html">导入管理员</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="output_dev.php">显示配件表</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="output_type.php">显示车型表</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<br></br>
                              <center>共<?php echo $num; ?>个消息&nbsp;&nbsp;&nbsp;
			</div>
			<table width="960" border="1">
				<tr>
					<th>编 号</th>
					<th>车 辆 名</th>
					<th>车 型 id</th>
					<th>登 记 时 间</th>
					<th>状 态</th>
				</tr>
				<?php
				
	
				foreach ($data as $key => $value) {
  					foreach ($value as $k => $v) {
    					$arr[$k] = $v;
  					}
  				echo "<tr>";
				echo "<td>{$arr[Car_ID]}</td>";
				echo "<td>{$arr[Car_Name]}</td>";
				echo "<td>{$arr[Type_ID]}</td>";
				echo "<td>{$arr[Car_Time]}</td>";
				echo "<td>{$arr[Car_Satus]}</td>";
				echo "<td>
							<a href='javascript:del({$arr[Car_ID]})'>删除</a>
							<a href='editStudent.php?id={$arr[Car_ID]}'>修改</a>
					  </td>";
				echo "</tr>";
  				// echo "<pre>";
 				// print_r($arr);
  				// echo "</pre>";
  				
  				
				}
				$up=$page-1;
$dn=$page+1;
if($page>1)
{
print "<center><br><a href=$php_self?page=$up>上一页</a>";
}
else
{
echo"<center><br>上一页 ";
}
if($pageno>$num)
{
print"下一页";
}
else
{
print "<a href=$php_self?page=$dn>下一页</a>|n";
}
				// 关闭连接
				mysqli_close($link);
			

				
				
			?>

			</table>
		</div>

		<script type="text/javascript">
			function del(id) {
				if (confirm("确定删除这个消息吗？")) {
					window.location = "action_del.php?id=" + id;
				}
			}
		</script>



	</body>
</html>




