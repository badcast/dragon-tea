
async function tea_ui_signin() {
    let userId = document.getElementById("inputSignin").value;

    let req = { user_id: userId };
    await tea_request(req);
}
