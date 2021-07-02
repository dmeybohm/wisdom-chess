<template>
  <div class="move-with-score">
    <div>{{ move }}</div>
    <div>{{ score }}</div>
  </div>
</template>

<script>
export default {
  name: "MoveWithScore",
  props: {
    move: String,
    score: Number
  }
}
</script>

<style scoped>
.move-with-score
{
  display: flex;
  flex-direction: column;
  align-items: center;
  margin-bottom: .4rem;
}
</style>