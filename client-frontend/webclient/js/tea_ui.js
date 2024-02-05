
const FORM =
{
    FDialog: 0,
    FSigin: 1
};

var UI =
{
    forms: [],
    activeForm: 0
};

function setSignFormStatus(message) {
    let inputMsg = document.getElementById("signStatusBox");
    inputMsg.innerText = message;
}

async function tea_ui_signin(self) {
    let userId = document.getElementById("inputSignin").value;

    setSignFormStatus("Входим...");

    // Check and validate of the ID
    if (/^\d+$/.test(userId) == false) {
        setSignFormStatus("Инвалидное поле");
        return false;
    }

    self.disabled = true;

    let req = { user_id: userId };
    let responce = await tea_request(req);

    await waitOut(200);

    if (responce.ok)
        setSignFormStatus("Вход выполнен");
    else
        setSignFormStatus(responce.error);

    self.disabled = false;
}

function tea_ui_start() {

}
