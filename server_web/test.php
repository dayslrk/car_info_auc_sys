<?php
//连接数据库
include 'conn.php';

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
			margin-bottom: 20px;
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
			<div class="add">
				<a href="addStudent.html">添加车辆信息</a>&nbsp;&nbsp;&nbsp;
				<a href="searchStudent.html">查找车辆</a>&nbsp;
				<a href="authenticate.html">认证</a>&nbsp;
				<a href="addtype.html">更新车辆类型</a>&nbsp;
				<a href="adddev.html">添加配件</a>&nbsp;
				<br></br>
				<a href="up_carinfo.html">导入车辆表</a>&nbsp;
				<a href="up_typeinfo.html">导入车型表</a>&nbsp;
				<a href="up_devinfo.html">导入配件表</a>&nbsp;
				<br></br>
				共<?php echo $num; ?>个消息&nbsp;&nbsp;&nbsp;
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
				//编写查询sql语句
                include 'conn.php';
                $page_size =10;
$total_pages = ceil($num/$page_size);
// 通过GET方式获得客户端访问的页码
$current_page_number = isset($_GET['page_number'])?$_GET['page_number']:1;
if($current_page_number<1) {
 $current_page_number =1;
}
if($current_page_number>$total_pages){
 $current_page_number = $total_pages;
}
echo "要访问的页码为：".$current_page_number;
 
// 获取到了要访问的页面以及页面大小，下面开始分页
$begin_position = ($current_page_number-1)*$page_size;
$sql = "select * from car limit $begin_position,$page_size";
$result = mysqli_query($link,$sql);   

if (!$result) {
    exit('查询sql语句执行失败。错误信息：'.mysqli_error($link));  // 获取错误信息
}

    
echo "<table border='#CCF solid 1px'><th>Mysql Fixed Assets Table</th>";
echo "<tr><td>DbName</td><td>TableName</td><td>Last_update</td><td>n_Nows</td><td>Clustered_Index_Size</td><td>Sum_od_Other_Index_sizes</td></tr>";
while(($row = mysqli_fetch_row($result))){
 echo "<tr>";
 echo "<td>".$row[0]."</td>";
 echo "<td>".$row[1]."</td>";
 echo "<td>".$row[2]."</td>";
 echo "<td>".$row[3]."</td>";
 echo "<td>".$row[4]."</td>";
 echo "<td>
							<a href='javascript:del({$arr[Car_ID]})'>删除</a>
							<a href='editStudent.php?id={$arr[Car_ID]}'>修改</a>
					  </td>";
 echo "</tr>";
}
echo "</table>";
 
// 循环显示总页数
?>
<?php
echo '<a href="SlicePage.php?page_number=1">首页</a>&nbsp;&nbsp;';
for($i=1;$i<=$total_pages;$i++){
 echo '<a href="./SlicePage.php?page_number='.$i.'">第'.$i.'页</a>&nbsp;&nbsp;'; 
}
echo '<a href="index.php?page_number='.($current_page_number-1).'">上一页</a>&nbsp;&nbsp;';
echo '<a href="index.php?page_number='.($current_page_number+1).'">下一页</a>&nbsp;&nbsp;';
echo '<a href="index.php?page_number='.($total_pages).'">尾页</a>&nbsp;&nbsp;';
// 释放数据连接资源
                mysqli_free_result($result);
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




