$(".accordion").accordion();
let postBtn = document.querySelector("#newPostBtn");
let newPostText = document.querySelector("#newPostText");
let postsSection = document.querySelector("#PostContainer");

postBtn.addEventListener("click", () => {
  if (newPostText.value === undefined || newPostText.value.trim().length == 0) {
    alert("5555555555555555555555");
  } else {
    let cleanedText = newPostText.value.replace(/\s+/g, " ");
    let newPost = document.createElement("div");
    newPost.innerHTML = `<div class="post-header">
          <img src="../Images/4.png" alt="User Profile" class="profile-pic" />
          <div class="post-user-info">
            <h3>Anonymous User</h3>
            <p>seconds ago</p>
          </div>
        </div>
        <div class="post-content">
          <p>
            ${cleanedText}
          </p>
          <div class="add-comment">
            <input
              type="text"
              placeholder="Write a comment..."
              class="comment-input"
            />
            <button class="comment-button">Reply</button>
          </div>
        </div>`;
    newPost.classList.add("post");
    postsSection.insertBefore(newPost, postsSection.children[1]);
  }
});
