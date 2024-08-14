fn main {
    if 123 {}
}

layout {
    dir: "ltr"
    title: "my site name"
    description: "desc"
    author: "Max Base"

    background: red
    color: white
    margin-top: "40px"

    content: "In the name of god!"
    //content: "In the name of god!2222"

    box {
        content: "Hey, mate!"
        display: flex
        flex-direction: column
        //flex-direction: row

        span {
            background: green
            color: white
            content: "Hey, mate!"
        }

        span {
            background: orange
            color: black
            content: "Hey, mate!"
        }
    }
}
