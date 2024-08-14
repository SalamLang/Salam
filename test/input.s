fn main {
    
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

    box {
        content: "Hey, mate!"
        display: flex
        flex-direction: column

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
