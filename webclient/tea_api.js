const _netapi = {
   signin: "/api/auth.php",
   signup: "/api/signup.php",
   pipeMsg: "/api/messagedb.php",
   userInfo: "/api/user.php"
};

async function request(){
   // JSON-данные для отправки
const requestData = {
  user_id: 1913913913913
};

// URL, к которому будет выполнен GET-запрос
const url = "https://dragontea.lightmister.repl.co/api/auth.php";

// Опции для запроса
const requestOptions = {
  method: 'GET',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify(requestData) // Преобразуем JSON-данные в строку
};

// Выполняем запрос
  await fetch(url, requestOptions)
  .then(response => response.json()) // Распарсить ответ как JSON
  .then(data => {
    // Обработка данных, полученных в ответе
    console.log(data);
  })
  .catch(error => {
    // Обработка ошибок
    console.error('Произошла ошибка:', error);
  });
}

function teaSigIn(userid, userNickname){

}

function teaSignUp(userNickname){

}

function teaUser(userNickname){

}
