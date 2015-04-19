/* YoshiBB.user.js
 * This is the default script sent to the client
 * this will handle all the communication with
 * the server and the displaying of the forum.
 * TODO: Save posts for when you forget to log in, and redirect back.
 * TODO: Create an XHR fallback.
 */


/*
 Possible state manager:
 Create a AddState function that adds additional commands to replay
 And another PushState function that will "save" the state and start
 a new one, THere will be another another function that acts as
 the parser, it just replays the commands in the state.
 */
/* socket is the object that represents the connection with the server */
var socket;
/* This ain't gonna change, so let's put it into a variable */
var hostname = window.location.hostname;
/* The containr element is what holds everything */
var cont = document.getElementById("container");
/* Self explainatory */
var d1 = true; //Command debugging
var d2 = true; //Method debugging

var forumName = "YoshiBB";
/* Hold the username and the id of the session
 * It is all checked serverside so w/e, this just makes the clientside easier to write
 */
var user = {
    username: "Guest",
    id: 0
};
var navbarContent = [
    ["Home", "", false, ["hme1"]],
    ["Profile", "/profile", false, ["prof", 0]],
    ["Search", "search", true, ["srch"]],
    ["Messages", "/inbox", true, ["ibox"]],
    ["Settings", "/settings", true ["sett"]]
]; //Label, url, 'real' navigation (refresh)
var config = {
    postsperpage: 20
};
//Every time we request data, the result is added to this.
//When the user presses the back button, this is refed into the parse function
var curState = [];
/* This will hold a majority of the information that doesn't /actually/ change
 * that often and can be gotten from memory rather than the server.
 * This is actually one of the major ideas driving this forum software
 * along with using websockets to send /just/ the data and assembling
 * the layout clientside, having a cache clientside will also
 * serve to greatly reduce the bandwidth required to operate the forum.
 * 
 */
var clientCache = {
    boards: [],
    posts: [],
    users: [],
    pms: [], /* This is going to be the biggest, since PMs are uneditable */
    addBoard: function(bid, bname, bparent) {
        this.boards[bid] = {id: bid, name: bname, parent: bparent};
    },
    getBoard: function(bid) {
        return this.boards[bid];
    },
    getChildren: function(bid) {
        var children = [];
        for (var i = 1; i < this.boards.length; i++) {
            //Make sure the board is real before trying to access it's parent.
            if (this.boards[i] && this.boards[i].parent === bid) {
                children.push(this.boards[i].id);
            }
        }
        return children;

    },
    getChain: function(bid) {
        var chain = [];
        if (this.boards[bid]) {
            var b = this.boards[bid];
            while (b != null) {
                chain.push([b.id, b.name]);
                b = this.boards[b.parent];
            }
        } else {


        }
        return chain.reverse();
    }
};
/* This might be removed soon */
var laststate = "";
var sitename = "ISW";
/* This gets updated to be an object (for easy accessing)
 * that holds all the values of the current url state
 */
var hashes = {};
/* This value tells the Init funtion which method to use */
var sockets = window.WebSocket;
/* This might be removed, it was supposed to keep track of the state of the connection */
var closed = true;
/* The database */
/* Some shorthands */
String.prototype.lpad = function(padString, length) {
    var str = this;
    while (str.length < length) {
        str = padString + str;
    }
    return str;
}
/* This is a chainable shorthand for appending children */

function appendChild(parent, newchildren) {
    if (!(Object.prototype.toString.call(newchildren) === '[object Array]'))
        return parent.appendChild(newchildren);
    else
        for (var i = 0, l = newchildren.length; i < l; i++) {
            parent.appendChild(newchildren[i]);
        }
    return parent;
}
window["appendChild"] = appendChild;
function log(str) {
    //str = null;
    console.log(str);
//str = null; /* Use this if want to remove from compiled version */
}
function log2(str) {
    //str = null;
    console.log(str);
//str = null; /* Use this if want to remove from compiled version */
}
/*A shorthand that will prove to reduce code length, and is hopefully inlined by the JS interpreter*/
function createElement(element) {
    return (document.createElement(element));
}/* function createElement(element) */
function getElement(id) {
    return document.getElementById(id);
}
window["createElement"] = createElement;
function addClass(element, className) {
    element.className = (element.className + " " + className).trim();
}
window["addClass"] = addClass;
function removeClass(element, className) {
    element.className = element.className.replace(className, _blank).replace(__, _blank).trim();
//element.className;
}
window["removeClass"] = removeClass;
//This is to prevent memory leaks.
function clearContents(element) {
    while (element.childNodes.length > 0) {
        element.removeChild(element.childNodes[0]);
    }
}
/* I stole this from this site, it was written in PHP, now it's not.
 * http://www.ultramegatech.com/2009/04/creating-a-bbcode-parser/
 * I regret nothing
 */
var srch = [
    /\[b\](.*?)\[\/b\]/gi, /* Bold ([b]text[/b] */
    /\[i\](.*?)\[\/i\]/gi, /* Italics ([i]text[/i] */
    /\[u\](.*?)\[\/u\]/gi, /* Underline ([u]text[/u]) */
    /\[s\](.*?)\[\/s\]/gi, /* Strikethrough ([s]text[/s]) */
    /\[quote\](.*?)\[\/quote\]/gi, /* Quote ([quote]text[/quote]) */
    /\[code\](.*?)\[\/code\]/gi, /* Monospaced code [code]text[/code]) */
    /\[size=([1-9]|1[0-9]|20)\](.*?)\[\/size\]/gi, /* Font size 1-20px [size=20]text[/size]) */
    /\[color=\#?([A-F0-9]{3}|[A-F0-9]{6})\](.*?)\[\/color\]/gi, /* Font color ([color=#00F]text[/color]) */
    /\[url=((?:ftp|https?):\/\/.*?)\](.*?)\[\/url\]/gi, /* Hyperlink with descriptive text ([url=http://url]text[/url]) */
    /\[url\]((?:ftp|https?):\/\/.*?)\[\/url\]/gi, /* Hyperlink ([url]http://url[/url]) */
    /\[img\](https?:\/\/.*?\.(?:jpg|jpeg|gif|png|bmp))\[\/img\]/gi, /* Image ([img]http://url_to_image[/img]) */
    /\n/gi,
    /\n\r/gi
];
/* T matching array of strings to replace matches withhe */
var repls = [
    '<strong>$1</strong>',
    '<em>$1</em>',
    '<u>$1</u>',
    '<del">$1</del>',
    '<blockquote>$1</blockquote>',
    '<pre>$1</pre>',
    '<span style="font-size: $1px;">$2</span>',
    '<span style="color: #$1;">$2</span>',
    '<a href="$1">$2</a>',
    '<a href="$1">$1</a>',
    '<img src="$1" alt="" />',
    '<br/>',
    '<br/>'
];
/* This function (should) protect against XSS attacks */
//This effectively sanitizes input.
function bbcParse(str, p) {
    if (str && str.replace) {

        /* First we remove HTML entities */
        str = str.replace(/[\<]/g, "&lt;").replace(/[\>]/g, "&gt;");
        /* If the p variable exists we have to parse it for bbcode */
        if (p) {
            for (var i = 0, l = repls.length; i < l; i++) {
                str = str.replace(srch[i], repls[i]);
            }
            /* Gonna have to write my own parser for moar stuff. */
        }
        return str;
    }
    else {
        return str;
    }/* if(str && str.replace) */
}/* function bbcParse(str,p) */
/* This is for the timestamp function which takes a timestamp, and returns the string for it */
var months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
function timestamp(dte, sht) {
    if (!sht) {
        return dte.toLocaleString();
        //return dte.getDate() + ',' + months[dte.getMonth()] + ' ' + dte.getFullYear() + ' ' + String(dte.getHours()).lpad("0", 2) + ':' + String(dte.getMinutes()).lpad("0", 2);
    } else {
        return (dte.getMonth() + 1) + "." + dte.getDate() + "." + String(dte.getFullYear()).slice(2, 4);
    } /* if(!sht) */

} /* function timestampd(dte,sht) */
function onclicker(arr) {
    return function() {
        send(socket, arr);
    }
}
function send(sock, array) {
    /* At one point, this will have a point that will access the localstorage, and call the appropriate functions.
     * then it will send a revised request to the server.
     */
    /* Idea 1: I will preparse the send array for specific commands, then I will check
     * the localstorage and call the parsecommand with what I have, then I will
     * update the array to reflect what I now need to do.
     * Things that can be held in localStorage:
     * Poster Info: Name, Join date, postcount (To be updated), siggy (to be updated)
     * Threads: Thread Subject, Poster
     * Posts: Subject, Content, Posttime (to be updated)
     */
    log2("Sending: " + JSON.stringify(array));
    /*
     * Aww hell naw.
     * This is actually the root of a major security concern for me.
     * How do I prevent this from crashing the server?
     */

    if (!socket || socket.readyState == 3) {
        log("Resuming connection");
        Init(true);
        setTimeout(function() {
            send(["hi"]);
            setTimeout(function() {
                sock.send(JSON.stringify(array));
            }, 500);
        }, 500);
    } else {
        sock.send(JSON.stringify(array));
        log("Sent [" + JSON.stringify(array) + "]");
    }
}
/* This function creates the div for the group of boards 
 * [id of group, group name]
 * 
 */
function MakeForumGroup(arr, cb) {
    if (d2)
        log("Making group(" + arr[0] + ") " + arr[1]);
    var group = createElement("div");
    group.id = "Group_" + arr[0];
    addClass(group, "ForumGroup");
    var groupName = createElement("div");
    groupName.id = "GroupName_" + arr[0];
    groupName.innerHTML = bbcParse(arr[1]);
    appendChild(cont, appendChild(group, [groupName]));
    if (!cb)
        clientCache.addBoard(arr[0], arr[1], 0);
}
/* This adds the child boards to the group div, and handles children
 * It's passed an array of arrays
 * [id of board,Name of board,Desc ,topics,posts,[last post id,poster id,username,subject,date],[child board id,child name]]
 * 
 */
function AddChildBoard(arr, cb) {
    var mySec = getElement("Group_" + arr[0]);
    for (var i = 1; i < arr.length; i++) {
        var subarr = arr[i];
        var bord = createElement("div");
        bord.id = "Board_" + subarr[0];
        addClass(bord, "Board");

        var nameDesc = createElement("div");
        addClass(nameDesc, "descholder");
        nameDesc.id = "BoardName_" + subarr[0];

        var bordName = createElement("span");
        addClass(bordName, "boardname link");
        bordName.innerHTML = bbcParse(subarr[1]);
        bordName.onclick = onclicker(["bord", subarr[0]]);

        var bordDesc = createElement("span");
        addClass(bordDesc, "boarddesc");
        bordDesc.innerHTML = bbcParse(subarr[2]);

        var postsInfo = createElement("div");
        addClass(postsInfo, "postinfo");

        var numTopics = createElement("span");
        numTopics.innerHTML = "Topics: " + subarr[3];

        var numPosts = createElement("span");
        numPosts.innerHTML = "Posts:" + subarr[4];

        var lastPostInfo = createElement("div");
        addClass(lastPostInfo, "lastpostinfo");

        if (subarr[5] && subarr[5][0] > 0) {
            var lastpostLabel = createElement("span");
            lastpostLabel.innerHTML = "Lastpost:";
            var lastpostTitle = createElement("span");
            lastpostTitle.innerHTML = bbcParse(subarr[5][3]);
            lastpostTitle.onclick = onclicker(["post", subarr[5][0]]);
            var posterName = createElement("span");
            posterName.innerHTML = bbcParse(subarr[5][2]);
            posterName.onclick = onclicker(["prof", subarr[5][1]]);
            var postDate = createElement("span");
            postDate.innerHTML = bbcParse(timestamp(new Date(subarr[5][4] * 1000)));
            appendChild(lastPostInfo, [lastpostLabel, lastpostTitle, createElement("br"), posterName, createElement("br"), postDate]);
        } else {
            var lastpost = createElement("span");
            lastpost.innerHTML = "No Posts";
            appendChild(lastPostInfo, lastpost);
        }


        appendChild(mySec,
                appendChild(bord, [/*newPost,*/
                    appendChild(nameDesc, [bordName, createElement("br"), bordDesc]),
                    appendChild(postsInfo, [numTopics, createElement("br"), numPosts]),
                    lastPostInfo
                ]
                        )
                );
        //If there are subboards.
        if (subarr[6]) {
            // This is where we add the child boards.
            var childBoardDiv = createElement("div");
            addClass(childBoardDiv, "childdiv");
            var childLabel = createElement("span");
            childLabel.innerHTML = "Children: ";
            appendChild(childBoardDiv, childLabel);
            for (var c = 6; c < subarr.length; c++) {
                var childLink = createElement("span");
                addClass(childLink, "link");
                childLink.innerHTML = bbcParse(subarr[c][1]);
                childLink.onclick = onclicker(["bord", subarr[c][0]]);
                appendChild(childBoardDiv, childLink);
                if (c != subarr.length - 1) {
                    var coma = createElement("span");
                    coma.innerHTML = " , ";
                    appendChild(childBoardDiv, coma);
                }
                if (!cb)
                    clientCache.addBoard(subarr[c][0], subarr[c][1], subarr[0]);
            }
            appendChild(bord, [createElement("br"), childBoardDiv]);
        }
        if (!cb)
            clientCache.addBoard(subarr[0], subarr[1], arr[0]);
    }


}
//This function creates each row of the forum
function MakeThread(arr) {

    //Grab the thread holder element
    var holder = getElement("threads");
    //Create the row itself
    var threadRow = createElement("tr");
    threadRow.id = "thread_" + arr[8];
    var subject = createElement("td");
    addClass(subject, "link");
    subject.innerHTML = bbcParse(arr[0]);
    subject.onclick = onclicker(["post", arr[8], 1]);
    var author = createElement("td");
    author.innerHTML = bbcParse(arr[1]);
    addClass(author, "link");
    author.onclick = onclicker(["prof", arr[2]]);
    var replies = createElement("td");
    replies.innerHTML = bbcParse(arr[3]);
    var lastpost = createElement("td");
    lastpost.innerHTML = bbcParse(arr[4]) + "@" + timestamp(new Date(arr[5] * 1000));


    appendChild(holder, appendChild(threadRow, [subject, author, replies, lastpost]));


}

function MakePageList(element, numpages, curpage, command) {
    //TODO: Add a document fragment shit
    if (numpages < 6) {
        for (var i = 1; i <= numpages; i++)

        {
            var s = createElement("span");
            s.innerHTML = i;
            addClass(s, "page link");
            appendChild(element, s);
            s.onclick = onclicker([command, parseInt(element.id.substring(11)), i, config.postsperpage]);
        }
    } else {
        for (var i = 1; i < 4; i++) {
            var s = createElement("span");
            s.innerHTML = i;
            addClass(s, "page link");
            appendChild(element, s);
            s.onclick = onclicker([command, parseInt(element.id.substring(11)), i, config.postsperpage]);
        }

//TODO: Check for the current page number
    }


}
//This will create the forum div that holds all the threads.
function MakeThreadGroup(id, curpage, numpages) {
    //Gonna make it wit tables

    var threadHolder = createElement("div");
    threadHolder.id = "threadHolder_" + id;
    addClass(threadHolder, "threadholder");
    var pageHolder = createElement("div");
    addClass(pageHolder, "pageholder");
    pageHolder.id = "pageholder_" + id;
    pageHolder.innerHTML = "Page:";
    MakePageList(pageHolder, numpages, curpage, "bord");
    //TODO: Make a standard function for page lists
    var buttonHolder = createElement("div");
    buttonHolder.id = "buttonholder_" + id;
    addClass(buttonHolder, "buttonholder");
    var postButton = createElement("span");
    addClass(postButton, "link");
    postButton.innerHTML = "New Topic";
    appendChild(buttonHolder, postButton);
    postButton.onclick = function() {
        MakeTopicForm(id);
    };
    var threads = createElement("table");
    addClass(threads, "threads");
    threads.id = "threads";
    var headers = createElement("tr");
    addClass(headers, "threadheaders");
    var subHeader = createElement("th");
    subHeader.innerHTML = "Subject";
    addClass(subHeader, "subjectheader");
    var posterHeader = createElement("th");
    posterHeader.innerHTML = "Author";
    addClass(posterHeader, "posterheader");
    var repliesHeader = createElement("th");
    repliesHeader.innerHTML = "Replies";
    addClass(repliesHeader, "repliesheader");
    var lastHeader = createElement("th");
    lastHeader.innerHTML = "Last Post";

    addClass(lastHeader, "lastheader");
    appendChild(headers, [subHeader, posterHeader, repliesHeader, lastHeader]);

    appendChild(threads, [headers]);
    appendChild(threadHolder, [pageHolder, buttonHolder, createElement("br"), threads]);
    appendChild(cont, threadHolder);

}
//This adds a single post onto the thread.
function MakePost(arr) {

    var postDiv = createElement("div");
    addClass(postDiv, "Post");
    postDiv.id = "post_" + arr[0]; //I'm a trashboat


    //Some of the code puts the id at the end, and some at the start
    //I really need to get around to refactoring the entire thing.
    //But I'll wait till I have it working again.



    var subjectBar = createElement("div");
    addClass(subjectBar, "subjectbar");

    var sub = createElement("span");
    sub.innerHTML = bbcParse(arr[3]);
    var replyBut = createElement("span");
    replyBut.innerHTML = "Reply";
    addClass(replyBut, "link");
    replyBut.onclick = function() {
        var topicid = parseInt(cont.getAttribute("thread"));
        var threadsub = cont.firstElementChild.firstElementChild.firstElementChild.innerHTML;
        MakeTopicForm(topicid, true);
        getElement("subject_create").value = threadsub;
    };

    var posterDiv = createElement("div");
    addClass(posterDiv, "UserInfo");
    var pdv = createElement("div");
    var posterIcon = createElement("img");
    addClass(posterIcon, "Avatar");
    var posterName = createElement("span");
    posterName.innerHTML = bbcParse(arr[5]);
    var posterTitle = createElement("span");
    posterTitle.innerHTML = "Witty title";
    var posterCount = createElement("span");
    posterCount.innerHTML = "0 posts";
    var posterDate = createElement("span");
    posterDate.inerHTML = "joined 4ever";


    var postContent = createElement("div");

    addClass(postContent, "PostData");
    var postBody = createElement("span");
    postBody.innerHTML = bbcParse(arr[4], 1);
    var postSig = createElement("span");
    addClass(postSig, "siggy_" + arr[1]);
    postSig.innerHTML = "whhwhwhwhw";
    appendChild(pdv, [posterIcon, createElement("br"), posterName, createElement("br"),
        posterTitle, createElement("br"), posterCount, createElement("br"), posterDate]);
    appendChild(postDiv, [appendChild(subjectBar, [sub, replyBut]), appendChild(posterDiv, [pdv]), appendChild(postContent, [postBody, createElement("br"), postSig])]);
    appendChild(cont, postDiv);

}
//This will make a profile given the array of stuff.
function MakeProfile(arr) {
    var cont = getElement("container");
    clearContents(cont);
    var thingy = createElement("div");
    addClass(thingy, "profilediv");
    var avatar = createElement("img");
    avatar.id = "profile_avatar";


    var datablock = createElement("div");
    datablock.id = "profile_data";
    var name = createElement("span");
    name.innerHTML = bbcParse(arr[0]);
    appendChild(datablock, [name, createElement("br")]);
    appendChild(thingy, [avatar, datablock]);
    appendChild(cont, thingy);


}
function MakeTopicForm(forum_id, isPost) {

    clearContents(cont);
    var formHolder = createElement("form");
    formHolder.id = "PostCreator";
    formHolder.method = "post";
    formHolder.target = "_self";
    formHolder.action = "";
    var subLab = createElement("label");
    subLab.innerHTML = "Subject";
    var subjectBar = createElement("input");
    subjectBar.id = "subject_create";
    var bodLab = createElement("label");
    bodLab.innerHTML = "Body";
    var bodyBar = createElement("textarea");
    bodyBar.id = "body_create";


    var subBut = createElement("input");
    subBut.id = "TopicSubmit";
    subBut.type = "submit";
	subBut.class = "btn";
    formHolder.onsubmit = function() {
        if (!isPost)
            send(socket, ["cpost1", forum_id, subjectBar.value, bodyBar.value]);
        else
            send(socket, ["cpost2", forum_id, subjectBar.value, bodyBar.value]);

        return false;
    };
    appendChild(formHolder, [subLab, createElement("br"), subjectBar, createElement("br"), bodLab, createElement("br"), bodyBar
                , createElement("br"), subBut]);


    appendChild(cont, formHolder);


}
function MakeLoginPage(reg) {

    clearContents(cont);
    var holder = createElement("form");
    addClass(holder, "loginbox");
    holder.action = "";
    holder.method = "post";
    holder.target = "_self";
    var boxHeader = createElement("span");
    boxHeader.innerHTML = (reg != true) ? "Login" : "Register";
    addClass(boxHeader, "heading");
    var usernameLab = createElement("label");
    usernameLab.innerHTML = "Username";
    usernameLab.setAttribute("for", "username");
    var usernameBox = createElement("input");
    usernameBox.name = "username";
    usernameBox.placeholder = "Username";
    addClass(usernameBox, "fancy");
    var passwordLab = createElement("label");
    passwordLab.innerHTML = "Password";
    passwordLab.setAttribute("for", "password");
    var passwordBox = createElement("input");
    passwordBox.name = "password";
    passwordBox.setAttribute("type", "password");
    passwordBox.placeholder = "Password";
    addClass(passwordBox, "fancy");
    appendChild(holder, [boxHeader, createElement("br"), usernameLab, createElement("br"), usernameBox,
        createElement("br"), passwordLab, createElement("br"), passwordBox]);


    if (reg === true) {
        SetURL(["register"]);
        var plab2 = createElement("label");
        plab2.innerHTML = "Confirm Password"
        plab2.setAttribute("for", "password2");

        var pbox2 = createElement("input");
        pbox2.name = "password2";
        pbox2.setAttribute("type", "password");
        addClass(pbox2, "fancy");
        var emailLab = createElement("label");
        emailLab.innerHTML = "Email (not required)";
        emailLab.setAttribute("for", "email");
        var emailBox = createElement("input");
        emailBox.name = "email";
        emailBox.setAttribute("type", "email");
        emailBox.value = "";
        addClass(emailBox, "fancy");
        appendChild(holder, [createElement("br"), plab2, createElement("br"),
            pbox2, createElement("br"), emailLab, createElement("br"), emailBox]);
    } else {
        SetURL(["login"]);
    }
    var subBut = createElement("input");
    subBut.setAttribute("type", "submit");
    subBut.value = "Submit";
    if (reg === true) {
        holder.onsubmit = function() {
            send(socket, ["reg", usernameBox.value, passwordBox.value, emailBox.value]);
            return false;
        };

    } else {

        holder.onsubmit = function() {
            send(socket, ["login", usernameBox.value, passwordBox.value]);
            return false;
        };
    }
    appendChild(holder, [createElement("br"), subBut]);
    appendChild(cont, holder);

}
function MakeRegisterPage() {
    MakeLoginPage(true);
}
//if rebuilding is true, then we don't send the requests
//Because they are being fed to us through the state machine
function Parse(msg, rebuilding) {
    log("Parsing: " + JSON.stringify(msg));
    curState.push(msg);
    var responseCode = msg[0];
    switch (responseCode) {
        case "board":
            send(socket, ["bord", msg[1]]);
            break;
        case "hello":
            //This is what the server initially sends when we establish a connection.
            //Since we've just started a connection
            //We need to attempt to log in.
            if (!rebuilding)
                send(socket, ["login"]);
            break;
        case "login":
            if (d1)
                log("Logged in as " + msg[1] + "(" + msg[2] + ")");
            user.username = msg[1];
            user.id = msg[2];
            var userbox = getElement("userbox");
            clearContents(userbox);
            var usernameText = createElement("span");
            usernameText.innerHTML = "Logged in as: " + bbcParse(msg[1]);
            if (msg[2] > 0) { //If the id is greater than 0 we are logged in
                //Create the link to the inbox.
                var inboxText = createElement("span");
                addClass(inboxText, "link u");

                inboxText.innerHTML = "Inbox (" + msg[3] + ")";
                inboxText.onclick = onclicker(["ibox"]);
                //Add the elements to the box.
                appendChild(userbox, [usernameText, createElement("br"), inboxText]);
                SetURL([""]);
            } else {
                //If there we are not logged in, don't display the inbox link.

                var loginText = createElement("span");
                loginText.innerHTML = "Login";
                loginText.onclick = MakeLoginPage;

                addClass(loginText, "link rlbut");
                var registerText = createElement("span");
                registerText.innerHTML = "Register";
                registerText.onclick = MakeRegisterPage;
                addClass(registerText, "link rlbut");
                appendChild(userbox, [usernameText, createElement("br"), loginText, registerText]);
            }
            var state = GetState();
            if (!rebuilding)
                switch (state[0]) {
                    case "":
                        //Nothing in the url, so load the main forums
                        log("Nothing");
                        //Hme1 is the command to ask for the forum groups.

                        send(socket, ["hme1"]);
                        break;
                    case "login":
                        MakeLoginPage();
                        break;
                    case "register":
                        MakeRegisterPage();
                        break;
                    case "board":
                        send(socket, ["bord", parseInt(state[1])]); //TODO: Drop the parse int, make the server more accepting
                        break;
                    case "":
                    default:

                        //Nothing in the url, so load the main forums
                        log("Nothing");
                        //Hme1 is the command to ask for the forum groups.

                        send(socket, ["hme1"]);
                        break;
                }
            break;
        case "reg":
            if (msg[1] == 1) {

                MakeLoginPage();
                alert("Registration Successful");
            } else {
                alert(msg[2]);
            }

            break;
        case "hme1":
            if (!rebuilding) {
                StartState(msg, "YoshiBB", "/");
            }
            //We are at the homepage, so we get the forum groups first
            var nvb = getElement("mainnav");
            clearContents(nvb);
            clearContents(cont);
            var fetchList = [];
            for (var i = 1; i < msg.length; i++) {
                //The format is [id, name]
                MakeForumGroup(msg[i].slice(0, 2));
                for (var b = 2; b < msg[i].length; b++) {
                    log(msg[i][b]);
                    AddChildBoard([msg[i][0], msg[i][b]]);

                }
            }
            /* This may have a potential SQLi vulnerability
             * I use FIND_IN_SET() on the string that is
             * sent, and that might open it up to injection.
             */
            if (!rebuilding) { //Phased out.
                // send(socket, ["hme2", fetchList]);
            }
            break;
        case "hme2": //For when specific top boards were requested.
            for (var i = 1; i < msg.length; i++) {
                AddChildBoard(msg[i]);
            }
            break;
        case "bord":
            var urt = "/";
            if (msg[2] == 1) {
                urt = "/board/" + msg[1];
            }
            else {
                urt = "/board/" + msg[1] + "/page/" + msg[2];
            }
            if (!rebuilding) {
                StartState(msg, "YoshiBB", urt);
            }
            //This is when we view a board.
            log("Building board: " + msg[1]);

            clearContents(cont);
            if (clientCache.getChildren(msg[1]).length > 0 || (msg[5] && msg[5].length > 0)) {
                //Board has children
                MakeForumGroup([msg[1], "Sub-forums"], true);
                if (!msg[5] || msg[5].length == 0)
                    send(socket, ["hme2", "" + msg[1]]);
            }
            var chain = clientCache.getChain(msg[1]);

            var nvb = getElement("mainnav");
            clearContents(nvb);
            var homepage = createElement("span");
            addClass(homepage, "navbar link");
            homepage.onclick = onclicker(["hme1"]);
            homepage.innerHTML = forumName;
            appendChild(nvb, homepage);
            for (var i = 0; i < chain.length; i++) {
                var navlink = createElement("span");
                navlink.innerHTML = chain[i][1];

                if (i < chain.length - 1) {
                    addClass(navlink, "navbar link");
                    navlink.onclick = onclicker(["bord", chain[i][0]]);
                } else {
                    addClass(navlink, "navbar");


                }
                var spacer = createElement("span");
                addClass(spacer, "navbar spacer");
                spacer.innerHTML = " >> ";
                appendChild(nvb, [spacer, navlink]);
            }
            //Make the top threadcap.
            if (msg[4] && msg[4].length > 0) {
                MakeThreadGroup(msg[1], msg[2], msg[3]);

                for (var i = 0; i < msg[4].length; i++) {

                    //Each thread is a separate array.
                    MakeThread(msg[4][i]);
                }
            }
            if (msg[5] && msg[5].length > 0) {

                //for (var i = 0; i < msg[5].length; i++) {
                msg[5].unshift(msg[1]);
                AddChildBoard(msg[5]);
                //TODO: Render child boards.
                //}
            }
            break;
        case "post":
            // SetURL(["thread", msg[1]]);
            if (!rebuilding) {
                StartState(msg, "YoshiBB", "/thread/" + msg[1]);
            }
            clearContents(getElement("container"));
            getElement("container").setAttribute("thread", msg[1]);
            for (var i = 2; i < msg.length; i++) {
                MakePost(msg[i]);
            }
            break;
        case "cpost1":
        case "cpost2": //just for now.
            send(socket, ["post", msg[1]]);
            break;
        case "prof":
            if (!rebuilding) {
                StartState(msg, "YoshiBB", "/profile/" + msg[1]);
            }
            log("Prof " + JSON.stringify(msg));
            MakeProfile(msg[2]); //Extract the response and send it ovar.

            break;
        case "error":
            alert(msg[1]);
            break;
    }



}
function SetURL(arr) {
    var surl = "/";

    history.pushState("", "", surl);
}
//This function simply parses the url and returns the array
function GetState() {
    log("Get State Called");
    var hash = window.location.pathname.toLowerCase();
    var hashtab = hash.split("/");
    hashtab = hashtab.splice(1);


    log("State is:" + JSON.stringify(hashtab));
    return hashtab;
}
// These methods exist to push the states
function StartState(state, title, url) {
    history.pushState([state], title, url);
}
function PushState(replay) {
    var curSt = history.state;
    curSt.push(replay);
    
    history.replaceState(curSt);
}
function SetState(state) {
    for (var i = 0; i < state.length; i++)
        state[i] = state[i].join("=");
    state = state.join("&");
    //Never take the last one, because that one switches the page.
    history.pushState(curState.slice(0, curState.length - 2), null, "?" + state);
    curState = [curState[curState.length - 1]];
}
//This is called when the back button is pressed.
function StatePopped(event) {
    console.log(event);
    if (event.state) {
        for (var i = 0; i < event.state.length; i++) {
            console.log("Parsing commands again");
            Parse(event.state[i], true);
        }
    }


}
function BuildNavbar() {


    //document.getElementById("navbar").innerHTML = JSON.stringify(navbarContent);
    for (var i in navbarContent) {
        var nav = createElement("span");
        
        nav.innerHTML = navbarContent[i][0];
        if (!navbarContent[i][2]) {
            addClass(nav, "nav link");
            nav.onclick = onclicker(navbarContent[i][3]);

        }
        var spac = createElement("span");
        spac.innerHTML = " | ";
        if (i < navbarContent.length - 1)
            appendChild(getElement("navbar"), [nav, spac]);
        else
            appendChild(getElement("navbar"), [nav]);



    }
}
function Init(ref) {
    log("Opening Socket");
    try {
        BuildNavbar();
        if (sockets) { /* Going to move this code to seperate file eventually one for Websocket enabled browsers, and one for not */
            var host = "ws://" + hostname + ":12346";
            if (!socket) {
                socket = new WebSocket(host);
                window.onunload = quit;
                socket.onopen = new function(msg) {
                    log("Connection to server opened.");
                    closed = false;
                };
                socket.onmessage = function(msg) {
                    if (msg) {
                        Parse(JSON.parse(msg.data));
                    }
                };
                socket.onclose = function(msg) {
                    alert("Closing message:" + msg.data);
                    socket = null;
                    closed = true;
                    /* We've lost connection to the server. */
                    log("Connection closed");
                };
            } else {
                log("wut");
            }
        }
    } catch (ex) {
        alert("Excrement has hit the air conditioning");
        alert(ex);
    }
}
function quit() {
    socket.close();
    socket = null;
}
window["createElement"] = createElement;
window["Init"] = Init;
document.body.onload = Init;
window.addEventListener("popstate", function(e) {
    StatePopped(e);
});

