fn main {
	print "Hey", 123, 3.14

	if 1 {
		if 2 {
			if 3 {
				print 123
				
				return 333
			}
		}
	}
}

layout {
	dir "ltr"
	title "my site name"
	description "desc"
	author "Max Base"
	
	background red
	color white
	margin-top 40

	// content "In the name of god!2222"

	box {
		content "Hey, mate!"
		display flex
		flex-direction column
		// flex-direction row

		span {
			background green
			color white
			content "Hey, mate!"
		}

		span {
			background orange
			color black
			content "Hey, mate!"
		}
	}
	
	content "In the name of god!"
}
