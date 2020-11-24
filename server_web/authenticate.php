<?php
	//连接数据库
	include 'conn.php';

	//获取id
	$id = $_GET['id'];
    $num= $_GET['num'];

	//编写查询sql语句
$sql = "SELECT * FROM dev WHERE Dev_ID='{$id}'";
	//执行查询操作、处理结果集
	$result = mysqli_query($link, $sql);
	if (!$result) {
	    exit('查询sql语句执行失败。错误信息：'.mysqli_error($link));  // 获取错误信息
	}
	$data = mysqli_fetch_all($result, MYSQLI_ASSOC);
	if (!$data) {
		//输出提示，跳转到首页
		echo "没有该车辆！<br><br>";
		header('Refresh: 3; url=index.php');  //3s后跳转
		exit();
	}
	//将二维数数组转化为一维数组
	foreach ($data as $key => $value) {
	  foreach ($value as $k => $v) {
	    $arr[$k]=$v;
	  }
	}





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
			<h2>配件信息认证</h2>
			<div class="add">
				<form action="index.php" method="post" enctype="multipart/form-data">
					<table border="1">
						<tr>
							<th>配 件 id：</th>
							<td><input type="text" name="id" size="5" value="<?php echo $arr[Dev_ID] ?>" readonly="readonly"></td>
						</tr>
						<tr>
							<th>配 件 名：</th>
							<td><input type="text" name="name" size="25" value="<?php echo $arr[Dev_Name] ?>" readonly="readonly"></td>
                        </tr>
                       
						<tr>
							<th>所 属 列 表：</th>
							<td><input type="text" name="list" size="25" value="<?php echo $arr[DevList_ID] ?>" readonly="readonly"></td>
						</tr>
						<tr>
							<th>登 记 时 间：</th>
							<td><input type="text" name="time" size="25" value="<?php echo $arr[Dev_Time] ?>" readonly="readonly"></td>
                        </tr>
                        <tr>
							<th>出 场 商：</th>
							<td><input type="text" name="oem" size="25" value="<?php echo $arr[DEV_OEM] ?>" readonly="readonly"></td>
						</tr>
						<tr>
							<th>状 态：</th>
							<td>
							<input type="text" name="status" size="5" value="<?php echo $arr[Status] ?>" readonly="readonly">
							</td>
						</tr>
						
						
						
						<tr>
							<th></th>
							<td>
								<input type="button" onClick="javascript :history.back(-1);" value="返回">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
								<input type="submit" value="确定">
							</td>
						</tr>
     <tr>
    <?php system("./hmac $arr[Dev_Key] $num") ?>
    </tr>
					</table>
				</form>
			</div>
		</div>
	</body>
</html>







