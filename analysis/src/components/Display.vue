<template>
  <div class="searches">
    <div v-for="search in searches" :key="search.id">
      <Chessboard :search="search" v-once />
    </div>
  </div>
</template>

<script>
import Chessboard from "./Chessboard";

export default {
  name: "Display",
  components: {Chessboard},
  data: function() {
    return {
      searches: []
    }
  },
  mounted: function() {
    fetch('http://localhost:8000/api/fetch.php?object=searches')
        .then(data => data.json())
        .then(data => {
          this.searches = data;
        })
  }
}
</script>

<style scoped>
.searches {
  display: flex;
  justify-content: center;
  margin-right: 15px;
}
</style>