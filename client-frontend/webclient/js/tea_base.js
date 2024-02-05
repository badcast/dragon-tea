 
 async function waitOut(ms)
 {
    await new Promise(resolve => setTimeout(resolve, 2000));
 }