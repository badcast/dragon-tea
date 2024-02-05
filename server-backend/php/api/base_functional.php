 <?php

// Convert STRING ID -> INTEGER ID
// Server Version >= 1.1.0
function patch_convert_id($id)
{
    if(is_string($id))
    {
        $filteredValue = filter_var($id, FILTER_VALIDATE_INT);

        if (!($filteredValue === false)){
            return $filteredValue;
        }
    }

    return $id;
}

function check_valid_nickname($nickname)
{
    // Проверяем длину имени пользователя
    $len = mb_strlen($nickname);
    if ($nickname == null || $len < 3 || $len > 16) {
        return false;
    }

    // Проверяем, что имя пользователя содержит только алфавиты (латинские и кириллические)
    // и цифры, также что оно не начинается с цифры
    if (!preg_match('/^[a-zA-Zа-яА-Я][a-zA-Zа-яА-Я0-9]*$/u', $nickname)) {
        return false;
    }

    return true;
}



 ?>
