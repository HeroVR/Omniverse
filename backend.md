# **Omniverse backend interface** #
These interfaces are used for backend, MMO game server likely. Cause we develop and test in a sandbox enviroment, so you must replace all web2d.vr.yingxiong.com to d.vr.yingxiong.com in retail enviroment.
## http://web2d.vr.yingxiong.com/gateway/check_user
Usually game client commits player's info to game server. The server should verify player's infomation, make sure the player is valid, not faked.
The sample call from game-server is below:
``` 
http://web2d.vr.yingxiong.com/gateway/check_user?gameid=1&qrcode=190f00f16809e018dbafc49200b3d3da&sign=d299dd347f034a49779ecc5212e971cc
```
|Paramter name|Type and length|Required|Description|
|-|-|-|-|
|gameid|string(30)|yes|GameID as in UHvInterface::init|
|sign|string(64)|yes|Signature string for vertification of this call|
|signtype|string(10)|no|Should be MD5|
|qrcode|string(50)|yes|Omniverse internal trade number|

The return result is in json format:
|Result parameter|Type|Required|Description|
|-|-|-|-|
|retCode|string|yes|"0" meaning sucess|
|retMsg|string|yes|return message or errors|
|accountId|string|yes|Player's ID on Omniverse|
|nickName|string|yes|Player's nick name on Omniverse|
|tel|string|yes|Player's phone number|
|sign|string|yes|signature string| 
**********************************************************************
## In-game purechase callback
CP can configure in-game purchase callback URL on CP backend. When in-game purchase success, Omniverse backend will invoke this URL to confirm purchase result.
It's more secure than the client's notify message. If the game have a game server or backend, CP should implement purchase result on game server. Game server should 
verify the callback and send the virtual-item player purchased.

The parameters are sended as POST method, and parameters are listed as below:
|Parameter name|Type(Length)|Required|Description|
|-|-|-|-|-|
|retCode|string(40)|yes|purchase result code, '0' means success, otherelse is error code|
|retMsg|string(40)|yes|result message or error description|
|gameid|string(40)|yes|GameID (same as UHvInterface::init(...) parameter) |
|sign|string(64)|yes|signature|
|signtype|string(40)|no|default is MD5|
|price|string(10)|yes|purchase price, unit: same as IPCUser::nWallet|
|outtradeno|string(40)|yes|CP customized trade number, CP can combine virtual-item type in this parameter|
|intradeno|string(40)|no|Omniverse trade number. No this parameter if retCode != '0'.|
|paytime|string(20)|no|Purchase complete time, format is 'YYYY-MM-DD hh:mm:ss'. No this parameter if retCode != '0'.|
|paystatus|string(20)|no|Purchase process status(3 status: 'paying', 'success', 'failed'). No this parameter if retCode != '0'.|

CP should return 'success' if purchase complete, otherwise purchase failed. Ominiverse will try Max 7 times to invoke this URL after timeout.
**********************************************************************
## How to create signature
1. Sort all parameters except sign and signtype by character;
2. Catch up all sorted parameters string as GET-parameters;
3. Make MD5 of GET-parameter and GameKey;
4. Use the MD5 result as the signature.

The signature verification method is same with creation.  
For example:
1. Source parameters as below. (If verify signature, exclude sign and signtype parameter) 
```PHP
array(‘gameid=>’1,
      ’title’=>’apple’,
      ’price’=>’32.05’,
      ’outtradeno’=>’31634001365’
)
```
2. Sort parameters, and make a GET-parameter 
```
gameid=1&outtradeno=31634001365&price=32.05&title=apple
```
3. Make MD5 (GameKey = '8E6U3W6mF' for example)
```PHP
var sign = MD5(gameid=79b7ed12621&outtradeno=31634001365&price=32.05&title=applev8E6U3W6mF);
// sign should be '44115ea103b7b301b853b534e7818de6'
```
4. Use MD5 result as signature. A http GET url should like below. To verify signature, just compare the MD5 result to sign. 
```
http://url?gameid=1&outtradeno=31634001365&price=32.05&title=apple&sign=44115ea103b7b301b853b534e7818de6
```



 
    