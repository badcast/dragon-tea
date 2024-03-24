
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

async function teaUISignIn(self) {
    let userId = document.getElementById("inputSignin").value;

    setSignFormStatus("Входим...");

    // Check and validate of the ID (all chars is number)
    if (/^\d+$/.test(userId) == false) {
        setSignFormStatus("Инвалидное поле");
        return false;
    }

    // Disable active button element 
    self.disabled = true;

    let req = { user_id: userId };
    let responce = await tea_request(req);

    // Wait for initializing 
    await waitOut(100);

    if (responce.ok) {

        setSignFormStatus("Вход выполнен");
        
    }
    else
        setSignFormStatus(responce.error);

    // And undo disabled active button element
    self.disabled = false;
}

function tea_ui_start() {

}
