<?php
//连接数据库
include 'conn.php';

//获取id
$id = $_GET['id'];
//编写查询sql语句
$sql = "SELECT * FROM car WHERE Car_ID='{$id}'";
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

// echo "<pre>";
// var_dump($arr);
// echo "</pre>";

?>

<html>
	<head>
		<meta charset="UTF-8">
		<title>车辆信息管理系统</title>
		<style type="text/css">
			body {
				background-image: url(student.jpg);
				background-size: 100%;
			}

			.box {
				display: table;
				margin: 0 auto;
			}

			h2 {
				text-align: center;
			}

			.add {
				margin-bottom: 20px;
			}
		</style>
	</head>
	<body>
		<!--输出定制表单-->
		<div class="box">
			<h2>修改车辆信息</h2>
			<div class="add">
				<form action="action_editStudent.php" method="post" enctype="multipart/form-data">
					<table border="1">
						<tr>
							<th>编 号：</th>
							<td><input type="text" name="id" size="5" value="<?php echo $arr[Car_ID] ?>" readonly="readonly"></td>
						</tr>
						<tr>
							<th>车 辆　名：</th>
							<td><input type="text" name="name" size="25" value="<?php echo $arr[Car_Name] ?>"></td>
						</tr>
						<tr>
							<th>车 型 id：</th>
							<td>
							<select name="sex" >
									<option <?php if (!$arr[Type_ID]) { echo "selected" ; } ?> value="">--请选择--</option>
									<option <?php if ($arr[Type_ID]=="type_01" ) { echo "selected" ; } ?> value="type_01">type_01</option>
									<option <?php if ($arr[Type_ID]=="type_02" ) { echo "selected" ; } ?> value="type_02">type_02</option>
									<option <?php if ($arr[Type_ID]=="type_03" ) { echo "selected" ; } ?> value="type_03">type_03</option>
									<option <?php if ($arr[Type_ID]=="type_04" ) { echo "selected" ; } ?> value="type_04">type_04</option>
									<option <?php if ($arr[Type_ID]=="type_05" ) { echo "selected" ; } ?> value="type_05">type_05</option>
								</select>
							</td>
						</tr>
						<tr>
							<th>登 记 时 间：</th>
							<td><input type="text" name="age" size="25" value="<?php echo $arr[Car_Time] ?>" readonly="readonly"></td>
						</tr>
						<tr>
							<th>状 态：</th>
							<td>
							<input type="text" name="edu" size="5" value="<?php echo $arr[Car_Satus] ?>" >
							</td>
						</tr>
						
						
						
						<tr>
							<th></th>
							<td>
								<input type="button" onClick="javascript :history.back(-1);" value="返回">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
								<input type="submit" value="提交">
							</td>
						</tr>
					</table>
				</form>
			</div>
		</div>
	</body>
</html>







