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

async function tea_request(requestData) {
  const _netapi = {
    signin: "/api/auth.php",
    signup: "/api/signup.php",
    pipeMsg: "/api/messagedb.php",
    userInfo: "/api/user.php"
  };

  var host = "http://localhost:8000";//"https://dragontea.lightmister.repl.co";//
  let url = host + _netapi.signin;

  // Опции для запроса
  const requestOptions = {
    method: 'POST',
    mode: 'cors',
    body: JSON.stringify(requestData)
  };

  // Выполняем запрос
  await fetch(url, requestOptions)
    .then(response => response.text()) // Распарсить ответ как JSON
    .then(data => {
      // Обработка данных, полученных в ответе
      alert(data)
    })
    .catch(error => {
      // Обработка ошибок
      alert(error)
    });
}
