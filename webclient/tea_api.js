function tea_str_error(errcode) {
  const STATUS_OK = 0;
  const STATUS_ID_NO_EXIST = 1;
  const STATUS_INVALID_REQUEST_DATA = 2;
  const STATUS_INVALID_AUTH_METHOD = 3;
  const STATUS_INVALID_NICKNAME = 4;
  const STATUS_INVALID_REGISTER = 5;
  const STATUS_ADMIN_ACCOUNT_REACHABLE = 6;

  const STATUS_PRIVATE_MESSAGE_NOT_SUPPORTED = 128;

  const STATUS_INTERNAL_SERVER_ERROR = 500;

  switch (errcode) {
    case STATUS_OK:
      return "ok";
    case STATUS_ID_NO_EXIST:
      return "user not exists";
    case STATUS_INVALID_REQUEST_DATA:
      return "invalid request data";
    case STATUS_INVALID_AUTH_METHOD:
      return "invalid auth method";
    case STATUS_INVALID_NICKNAME:
      return "invalid nickname";
    case STATUS_INVALID_REGISTER:
      return "invalid register";
    case STATUS_ADMIN_ACCOUNT_REACHABLE:
      return "admin account reachable";
    case STATUS_PRIVATE_MESSAGE_NOT_SUPPORTED:
      return "private message not supported";
    case STATUS_INTERNAL_SERVER_ERROR:
      return "internal server error";
  }
  return "";
}

var net_stats =
{
  requestVerified: 0,
  requestError: 0,
  uploadBytes: 0,
  downloadBytes: 0
};

var serverInfo =
{
  hasDamager: false
};

var userProfile =
{
  user: {
    user_id: 0,
    creation_date: 0,
    last_login: 0,
    user_nickname: ""
  }
};

async function tea_request(requestData) {
  const _netapi = {
    signin: "/api/auth.php",
    signup: "/api/signup.php",
    pipeMsg: "/api/messagedb.php",
    userInfo: "/api/user.php"
  };

  var host = "http://localhost:8000";//"https://dragontea.lightmister.repl.co";//
  let url = host + _netapi.signin;

  let bodyStr = JSON.stringify(requestData);
  console.log(bodyStr);

  // Опции для запроса
  const requestOptions = {
    method: 'POST',
    mode: 'cors',
    body: bodyStr
  };

  // Выполняем запрос
  let status = await fetch(url, requestOptions).then(response => response.text()) // Распарсить ответ как JSON
    .then(data => {
      let status = false;
      let errStr = null;
      net_stats.requestVerified++;
      while (1) {
        if (data == "") {
          errStr = "Internal server error. Responce is empty.";
          break;
        }

        // Get index of the JSON data
        let fIndex = data.indexOf("{");
        if ((serverInfo.hasDamager = fIndex != 0)) {
          if (fIndex === -1) {
            errStr = "Server is force damager";
            break;
          }
          // remove damaged 
          data = data.slice(fIndex, data.length);
        }

        // Convert string data to json object 
        data = JSON.parse(data);

        if (data.status != 0) {
          errStr = tea_str_error(data.status);
          break;
        }

        if (data.authorized) {
          status = true;
        }
        break;
      }

      if (status) {

      }

      return { error: errStr, ok: status }
    })
    .catch(error => {
      // Обработка ошибок
      console.error(error);
      net_stats.requestError++;
      return { error: error, ok: false };
    });

  if (status.ok) {
    userProfile.user = status.result;
  }
  else {
    console.error(status.error);
  }
  return status;
}
