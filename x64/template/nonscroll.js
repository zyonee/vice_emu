window.onload= resizeSplitWndw;
window.onresize= resizeSplitWndw;
window.onbeforeprint= set_to_print;
window.onafterprint= reset_form;

function resizeSplitWndw(){

if(navigator.userAgent.indexOf("MSIE")>-1){

var onsr= document.getElementById("nsr");
var omainbody= document.getElementById("mainbody");

if(document.body.clientWidth>0 && document.body.clientHeight>onsr.clientHeight){
document.getElementById("mainbody").style.width= 0; //IE6 reload bug
if (omainbody ==null) return;
if (onsr != null){
omainbody.style.overflow= "auto";
onsr.style.width= document.body.offsetWidth-4;
omainbody.style.width= document.body.offsetWidth-4;
omainbody.style.top= onsr.offsetHeight;
if (document.body.offsetHeight > onsr.offsetHeight)
omainbody.style.height= document.body.offsetHeight - onsr.offsetHeight-4;
else omainbody.style.height=0;
}
}
}
}

function set_to_print(){

var i, docAll;

if (window.mainbody)document.getElementById("mainbody").style.height = "auto";

docAll = document.getElementsByTagName("*");

for (i=0; i < docAll.length; i++){
if (docAll[i].tagName == "BODY") {
docAll[i].scroll = "auto";
}
if (docAll[i].tagName == "A") {
docAll[i].outerHTML = "<a href=''>" + docAll[i].innerHTML + "</a>";
}
}
}

function reset_form(){

document.location.reload();
}