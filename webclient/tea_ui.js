
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

async function tea_ui_signin() {
    let userId = parseInt(document.getElementById("inputSignin").value);

    let req = { user_id: userId };
    await tea_request(req);
}

function tea_ui_start(){
    alert("asdsadasd");
}

document.body.onload = function ( env ) { tea_ui_start(); };

void tea_ui_show(formId)
{

}